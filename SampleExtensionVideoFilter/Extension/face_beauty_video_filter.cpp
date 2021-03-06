#include "pch.h"
#include <sstream>
#include "face_beauty_video_filter.h"

#define ENUM_TO_STR(TYPE) std::string(#TYPE)

static std::map<std::string, xhs_Command_type> m_xhs_command_dict;
static void initCommandDict();

CFaceBeautyVideoFilter::CFaceBeautyVideoFilter(const char* id, const EngineInitParamsAid& config, agora::rtc::IExtensionControl* core)
  : enabled_(true), id_(id), config_(config), core_(core), init_(false) {
}

CFaceBeautyVideoFilter::~CFaceBeautyVideoFilter() {
    if (m_pBeautyEngine != nullptr)
    {
        m_pBeautyEngine->destroyWindowsEngine();
        delete m_pBeautyEngine;
        m_pBeautyEngine = nullptr;
    }
}

bool CFaceBeautyVideoFilter::onDataStreamWillStart() {
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
    if (m_pBeautyEngine == nullptr) {
        return false;
    }
    m_pBeautyEngine->processYUV(capturedFrame.yBuffer, capturedFrame.uBuffer, capturedFrame.vBuffer,capturedFrame.width, capturedFrame.height);
    adaptedFrame = capturedFrame;
    m_pBeautyEngine->getOutputYUVData(adaptedFrame.yBuffer, adaptedFrame.uBuffer, adaptedFrame.vBuffer);
    return true;
}

size_t CFaceBeautyVideoFilter::setProperty(const char* key, const void* buf, size_t buf_size) {
    //auto type = m_xhs_command_dict[key];
    auto it = m_xhs_command_dict.find(key);
    if (it == m_xhs_command_dict.end())
    {
        printf("invalid xhs command key: %s\n", key);
        return -1;
    }
    auto type = it->second;
    if (m_pBeautyEngine == nullptr || buf == nullptr)
    {
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
            return -1;
        }
        std::replace(aid._subPath.begin(), aid._subPath.end(), '/', '\\');
        m_pBeautyEngine->setFilterResourcePath(aid._subPath.c_str());
        m_pBeautyEngine->setFilterIntensity(aid._lut_intensity);
    }

    case XHS_PLUGIN_COMMAND_INVALID:
        printf("invalid command: %s\n", key);
        break;
    default:
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