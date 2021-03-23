#include "pch.h"
#include <sstream>
#include <windows.h>
#include <stdio.h>
#include "face_beauty_video_filter.h"

#define ENUM_TO_STR(TYPE) std::string(#TYPE)

static std::map<std::string, xhs_Command_type> m_xhs_command_dict;
static void initCommandDict();

CFaceBeautyVideoFilter::CFaceBeautyVideoFilter(const char* id, const EngineInitParamsAid& config, agora::rtc::IExtensionControl* core)
  : enabled_(true), id_(id), config_(config), core_(core), init_(false) {
  printf("this: %p, thread: %d, CFaceBeautyVideoFilter::CFaceBeautyVideoFilter, id: %s\n", this, ::GetCurrentThreadId(), id);
}

CFaceBeautyVideoFilter::~CFaceBeautyVideoFilter() {
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::~CFaceBeautyVideoFilter\n", this, ::GetCurrentThreadId());
    if (m_pBeautyEngine != nullptr)
    {
        m_pBeautyEngine->destroyWindowsEngine();
        delete m_pBeautyEngine;
        m_pBeautyEngine = nullptr;
        //printf("this: %p, thread: %d, CFaceBeautyVideoFilter::~CFaceBeautyVideoFilter, release beauty engine\n", this, ::GetCurrentThreadId());
    }
}

bool CFaceBeautyVideoFilter::onDataStreamWillStart() {
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::onDataStreamWillStart\n", this, ::GetCurrentThreadId());
    if (!init_)
    {
        initCommandDict();

        m_pBeautyEngine = new CG::XYCGWindowsEngine();
        int result = m_pBeautyEngine->initWindowsEngine(config_._license.c_str(), config_._userId.c_str());
        nlohmann::json result_json = result;
        if (core_) {
            core_->log(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO, "init xhs_filter_engine");
            core_->fireEvent(id_.c_str(), "INIT_XHS_FILTER_ENGINE", result_json.dump().c_str());
        }

        if (result != 0)
        {
            printf("Beauty engine init failed. Error code:%d\n", result);
            m_pBeautyEngine->destroyWindowsEngine();
            delete m_pBeautyEngine;
            m_pBeautyEngine = nullptr;
            return false;
        }

        // temp load ai here
        result = m_pBeautyEngine->loadAIModel(config_._aiModelPath.c_str());
        // temp load beauty image here
        result = m_pBeautyEngine->setBeautyResourcePath(config_._beautyResPath.c_str());
        init_ = true;
    }
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::onDataStreamWillStart finish, init: %d\n", this, ::GetCurrentThreadId(), init_);
    return true;
}

void CFaceBeautyVideoFilter::onDataStreamWillStop() {
    if (init_)
    {
        if (m_pBeautyEngine != nullptr) {
            int result = m_pBeautyEngine->destroyWindowsEngine();
            delete m_pBeautyEngine;
            m_pBeautyEngine = nullptr;
            init_ = false;
        }
    }
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::onDataStreamWillStop, init: %d\n", this, ::GetCurrentThreadId(), init_);
    return;
}

void initCommandDict() {
    std::pair<std::string, xhs_Command_type> pairs[XHS_PLUGIN_COMMAND_COUNT] = {
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_COMMAND_INVALID),      XHS_PLUGIN_COMMAND_INVALID),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_BEAUTY_FILTER_SWITCH), XHS_PLUGIN_BEAUTY_FILTER_SWITCH),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_BEAUTY_TYPE),          XHS_PLUGIN_BEAUTY_TYPE),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_COLOR_FILTER_SWITCH),  XHS_PLUGIN_COLOR_FILTER_SWITCH),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_COLOR_TYPE),           XHS_PLUGIN_COLOR_TYPE),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_LUT_FILTER_SWITCH),    XHS_PLUGIN_LUT_FILTER_SWITCH),
        std::pair<std::string, xhs_Command_type>(ENUM_TO_STR(XHS_PLUGIN_LUT_FILTER_TYPE),      XHS_PLUGIN_LUT_FILTER_TYPE)
    };

    m_xhs_command_dict = std::map<std::string, xhs_Command_type>(pairs, pairs + XHS_PLUGIN_COMMAND_COUNT);
}

bool CFaceBeautyVideoFilter::adaptVideoFrame(const agora::media::base::VideoFrame& capturedFrame,
    agora::media::base::VideoFrame& adaptedFrame) {
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::adaptVideoFrame, m_pBeautyEngine: %p\n", this, ::GetCurrentThreadId(), m_pBeautyEngine ? m_pBeautyEngine : 0);
    if (m_pBeautyEngine == nullptr) {
        return false;
    }
    m_pBeautyEngine->processYUV(capturedFrame.yBuffer, capturedFrame.uBuffer, capturedFrame.vBuffer,capturedFrame.width, capturedFrame.height);
    adaptedFrame = capturedFrame;
    m_pBeautyEngine->getOutputYUVData(adaptedFrame.yBuffer, adaptedFrame.uBuffer, adaptedFrame.vBuffer);
    return true;
}

size_t CFaceBeautyVideoFilter::setProperty(const char* key, const void* buf, size_t buf_size) {
    printf("this: %p, thread: %d, CFaceBeautyVideoFilter::setProperty, key: %s, buf: %s\n", this, ::GetCurrentThreadId(), key, (buf ? (char*)buf : "null"));
    //auto type = m_xhs_command_dict[key];
    auto it = m_xhs_command_dict.find(key);
    if (it == m_xhs_command_dict.end())
    {
        printf("this: %p, thread: %d, invalid xhs command key: %s\n", this, ::GetCurrentThreadId(), key);
        return -1;
    }
    auto type = it->second;
    if (m_pBeautyEngine == nullptr || buf == nullptr)
    {
        printf("this: %p, thread: %d, m_pBeautyEngine or buf null\n", this, ::GetCurrentThreadId());
        return -1;
    }
    switch (type)
    {
    case XHS_PLUGIN_BEAUTY_FILTER_SWITCH:
    {
        bool value = false;
        nlohmann::json j = nlohmann::json::parse((char*)buf);
        j.get_to(value);
        m_pBeautyEngine->enableBeauty(value);
    }
        break;

    case XHS_PLUGIN_BEAUTY_TYPE: 
    {
        BeautyFilterAid aid;
        aid.from_json(std::string{(char*)buf});
        m_pBeautyEngine->enableBeautyType(aid._type, aid._switch);
        if (aid._switch)
        {
            m_pBeautyEngine->setBeautyTypeIntensity(aid._type, aid._beauty_type_intensity);
        }
    }
    break;

    case XHS_PLUGIN_COLOR_FILTER_SWITCH:
    {
        bool value = false;
        nlohmann::json j = nlohmann::json::parse((char*)buf);
        j.get_to(value);
        m_pBeautyEngine->enableColorfulAdjustment(value);
    }
    break;

    case XHS_PLUGIN_COLOR_TYPE:
    {
        ColorFilterAid aid;
        aid.from_json(std::string{ (char*)buf });
        m_pBeautyEngine->setColorfulTypeIntensity(aid._type, aid._color_intensity);
    }
    break;

    case XHS_PLUGIN_LUT_FILTER_SWITCH:
    {
        bool value = false;
        nlohmann::json j = nlohmann::json::parse((char*)buf);
        j.get_to(value);
        m_pBeautyEngine->enableFilter(value);
    }
    break;

    case XHS_PLUGIN_LUT_FILTER_TYPE:
    {
        LutFilterAid aid;
        aid.from_json(std::string{ (char*)buf });

        if (aid._subPath.empty()) {
            std::ostringstream message;
            message << "invalid path for lut: " << aid._subPath;
            core_->log(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO, message.str().c_str());
            printf("this: %p, thread: %d, aid _subPath empty\n", this, ::GetCurrentThreadId());
            return -1;
        }
        std::replace(aid._subPath.begin(), aid._subPath.end(), '/', '\\');
        m_pBeautyEngine->setFilterResourcePath(aid._subPath.c_str());
        m_pBeautyEngine->setFilterIntensity(aid._lut_intensity);
    }

    case XHS_PLUGIN_COMMAND_INVALID:
        printf("this: %p, thread: %d, invalid command: %s\n",this, ::GetCurrentThreadId(), key);
        break;
    default:
        printf("this: %p, thread: %d, unknown command: %s, type: %d\n",this, ::GetCurrentThreadId(), key, type);
        break;
    }
  return -1;
}

xhs_Command_type getKeyCommandType(const char* key) {
    
    auto type = XHS_PLUGIN_COMMAND_INVALID;

    auto it = m_xhs_command_dict.find(key);
    if (it != m_xhs_command_dict.end())
    {
        type = m_xhs_command_dict[key];
    }

    return type;
}