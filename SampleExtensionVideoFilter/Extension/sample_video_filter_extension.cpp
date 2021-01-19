#include "pch.h"
#include "sample_video_filter_extension.h"

#define ENUM_TO_STR(TYPE) std::string(#TYPE)

static std::map<std::string, xhs_Command_type> m_xhs_command_dict;
static void initCommandDict();

CSampleVideoFilter::CSampleVideoFilter(const char* id, agora::rtc::IExtensionControl* core)
  : enabled_(true), id_(id), core_(core), init_(false) {
  

}

CSampleVideoFilter::~CSampleVideoFilter() {
    if (m_pBeautyEngine != nullptr)
    {
        m_pBeautyEngine->destroyWindowsEngine();
        delete m_pBeautyEngine;
        m_pBeautyEngine = nullptr;
    }
}

bool CSampleVideoFilter::onDataStreamWillStart() {
    if (!init_)
    {
        initCommandDict();

        m_pBeautyEngine = new CG::XYCGWindowsEngine();
        int result = m_pBeautyEngine->initWindowsEngine();
        if (result != 0)
        {
            printf("Beauty engine init failed. Error code:%d\n", result);
            m_pBeautyEngine->destroyWindowsEngine();
            delete m_pBeautyEngine;
            m_pBeautyEngine = nullptr;
        }

        // temp load ai here
        result = m_pBeautyEngine->loadAIModel(m_aiModelPath.c_str());
        // temp load beauty image here
        result = m_pBeautyEngine->setBeautyResourcePath(m_beautyResPath.c_str());
        //result = m_pBeautyEngine->setFilterResourcePath(m_lutResPath.c_str());
        init_ = true;
    }
    return true;
}

void CSampleVideoFilter::onDataStreamWillStop() {
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

bool CSampleVideoFilter::adaptVideoFrame(const agora::media::base::VideoFrame& capturedFrame,
    agora::media::base::VideoFrame& adaptedFrame) {
    
    
    m_pBeautyEngine->processYUV(capturedFrame.yBuffer, capturedFrame.uBuffer, capturedFrame.vBuffer,capturedFrame.width, capturedFrame.height);
    adaptedFrame = capturedFrame;
    m_pBeautyEngine->getOutputYUVData(adaptedFrame.yBuffer, adaptedFrame.uBuffer, adaptedFrame.vBuffer);
    if (core_) {
      core_->log(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO, "accept one video frame");
      core_->fireEvent(id_.c_str(), "test_key", "test_value");
    }
    return true;
}

size_t CSampleVideoFilter::setProperty(const char* key, const void* buf, size_t buf_size) {
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
        assert(buf_size == sizeof(BeautyFilterAid));
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
            printf("invalid path for lut: %s\n", aid._subPath);
            return -1;
        }

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

void CSampleVideoFilter::setEngineInitParams(const EngineInitParamsAid& aid)
{
    if (aid._license == "") {
        printf("warning: xhs beauty_filter_engine license path is empty.\n");
    } else {
        m_license = aid._license;
    }

    if (aid._userId == "") {
        printf("warning: xhs beauty_filter_engine userId is empty.\n");
    }
    else {
        m_userId = aid._userId;
    }

    if (aid._aiModelPath == ""){
        printf("warning: xhs beauty_filter_engine Ai_Model path is empty.\n");
    } else {
        m_aiModelPath = aid._aiModelPath;
    }

    if (aid._beautyResPath == "") {
        printf("warning: xhs beauty_filter_engine beauty_Res path is empty.\n");
    } else {
        m_beautyResPath = aid._beautyResPath;
    }

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