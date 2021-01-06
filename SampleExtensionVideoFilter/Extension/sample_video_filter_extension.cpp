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
    
    if (!init_)
    {
        initCommandDict();

        const char* key = "XHS_PLUGIN_BEAUTY_FILTER_SWITCH";
        xhs_Command_type type_ = getKeyCommandType(key);

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
        result = m_pBeautyEngine->loadAIModel("slim-320.face_kpt_v2.mouth.eyebrow.bin");
        // temp load image here
        m_pBeautyEngine->setBeautyResourcePath("Beauty_Res");
        m_pBeautyEngine->setFilterResourcePath("vsco6");
        init_ = true;
    }
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
    auto type = m_xhs_command_dict[key];
    if (m_pBeautyEngine == nullptr || buf == nullptr)
    {
        return -1;
    }
    switch (type)
    {
    case XHS_PLUGIN_BEAUTY_FILTER_SWITCH:
    {
        bool value = *(bool*)buf;
        m_pBeautyEngine->enableBeauty(value);
    }
        break;

    case XHS_PLUGIN_BEAUTY_TYPE: 
    {
        assert(buf_size == sizeof(BeautyFilterAid));
        BeautyFilterAid* aid = (BeautyFilterAid*)buf;
        auto type = aid->_type;
        auto turn = aid->_switch;
        auto value = aid->_beauty_type_intensity;
        m_pBeautyEngine->enableBeautyType(type, turn);
        if (turn)
        {
            m_pBeautyEngine->setBeautyTypeIntensity(type, value);
        }
    }
        break;

    case XHS_PLUGIN_COLOR_FILTER_SWITCH:
    {
        bool value = *(bool*)buf;
        m_pBeautyEngine->enableColorfulAdjustment(value);
    }
        break;

    case XHS_PLUGIN_COLOR_TYPE:
    {
        assert(buf_size == sizeof(ColorFilterAid));
        ColorFilterAid* aid     = (ColorFilterAid*)buf;
        xhs_colorful_type type  = aid->_type;
        float value             = aid->_color_intensity;
        m_pBeautyEngine->setColorfulTypeIntensity(type, value);
    }
        break;

    case XHS_PLUGIN_LUT_FILTER_SWITCH:
    {
        bool value = *(bool*)buf;
        m_pBeautyEngine->enableFilter(value);
    }
        break;

    case XHS_PLUGIN_LUT_FILTER_TYPE:
    {
        assert(buf_size == sizeof(LutFilterAid));
        LutFilterAid* aid = (LutFilterAid*)buf;

        if (aid->_subPath == nullptr) {
            printf("invalid path for lut: %s\n", aid->_subPath);
            return -1;
        }

        std::string subPath = aid->_subPath;
        float value         = aid->_lut_intensity;
        m_pBeautyEngine->setFilterResourcePath(subPath.c_str());
        m_pBeautyEngine->setFilterIntensity(value);
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