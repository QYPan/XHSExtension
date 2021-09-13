#include "pch.h"
#include <sstream>
#include <fstream>
#include <map>
#include <windows.h>
#include <stdio.h>
#include "face_beauty_video_filter.h"

namespace agora {
namespace extension {

#define ENUM_TO_STR(TYPE) std::string(#TYPE)

static std::map<std::string, xhs_Command_type> m_xhs_command_dict;
static void initCommandDict();

class SimpleLogger {
public:
    static SimpleLogger* GetInstance() {
        if (!instance_) {
            instance_ = new SimpleLogger();
        }
        return instance_;
    }

    static void DestroyInstance() {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    enum LOG_TYPE {
        L_INFO,
        L_WARN,
        L_ERROR,
        L_FUNC
    };

    void Print(LOG_TYPE type, const char* fmt, ...) {
        if (!fmt || !*fmt) {
            return;
        }

        va_list ap;
        va_start(ap, fmt);
        auto size = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (size <= 0) {
            return;
        }

        std::unique_ptr<char[]> buf = std::make_unique<char[]>(size + 2);
        memset(buf.get(), 0, size + 2);
        va_start(ap, fmt);
        size = vsnprintf(buf.get(), size + 2, fmt, ap);
        va_end(ap);
        if (size <= 0) {
            return;
        }

        static std::map<LOG_TYPE, std::string> log_type_map = {{L_INFO, "[I]:"},{L_WARN, "[W]:"},{L_ERROR, "[E]:"}, {L_FUNC, "[F]:"}};
        SYSTEMTIME sysTime = { 0 };
        GetLocalTime(&sysTime);
        char strtime[64] = { 0 };
        _snprintf_s(strtime, 64, " [%04d/%02d/%02d %02d:%02d:%02d:%02d] ", sysTime.wYear, sysTime.wMonth, sysTime.wDay,
            sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

        std::string msg(buf.get());
        if (writer_.is_open()) {
            writer_ << log_type_map[type] << strtime << "[" << ::GetCurrentThreadId() << "] " << msg << std::endl;
        }
    }

private:
    SimpleLogger() {
        writer_.open("face_beauty_video_filter.log", std::ofstream::out);
    }

    ~SimpleLogger() {
        writer_.close();
    }

    static SimpleLogger *instance_;
    std::ofstream writer_;
};

SimpleLogger* SimpleLogger::instance_ = nullptr;

#define PRINT_LOG(type, ...) \
	SimpleLogger::GetInstance()->Print(type, ##__VA_ARGS__)

CFaceBeautyVideoFilter::CFaceBeautyVideoFilter(const char* id, const EngineInitParamsAid& config, agora::rtc::IExtensionControl* core)
  : id_(id), config_(config), core_(core), init_(false) {
    PRINT_LOG(SimpleLogger::LOG_TYPE::L_FUNC, "%s: this: %p, id: %s.", __FUNCTION__, this, id);
}

CFaceBeautyVideoFilter::~CFaceBeautyVideoFilter() {
    PRINT_LOG(SimpleLogger::LOG_TYPE::L_FUNC, "%s: this: %p, engine: %p.", __FUNCTION__, this, m_pBeautyEngine);
    if (m_pBeautyEngine != nullptr)
    {
        m_pBeautyEngine->destroyWindowsEngine();
        delete m_pBeautyEngine;
        m_pBeautyEngine = nullptr;
    }
}

int CFaceBeautyVideoFilter::start(agora::agora_refptr<Control> control) {
  PRINT_LOG(SimpleLogger::LOG_TYPE::L_FUNC, "%s: this: %p, init: %d.", __FUNCTION__, this, init_);
  if (!init_)
    {
        initCommandDict();

        m_pBeautyEngine = new CG::XYCGWindowsEngine();
        int result = m_pBeautyEngine->initWindowsEngine(config_._license.c_str(), config_._userId.c_str());
        nlohmann::json result_json = result;
        if (core_) {
            core_->log(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO, "init xhs_filter_engine");
            core_->fireEvent("xiaohongshu", id_.c_str(), "INIT_XHS_FILTER_ENGINE", result_json.dump().c_str());
        }

        if (result)
        {
            PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, beauty engine init failed. result: %d.", this, result);
            m_pBeautyEngine->destroyWindowsEngine();
            delete m_pBeautyEngine;
            m_pBeautyEngine = nullptr;
            return -1;
        }

        // temp load ai here
        result = m_pBeautyEngine->loadAIModel(config_._aiModelPath.c_str());
        if (result) {
            PRINT_LOG(SimpleLogger::LOG_TYPE::L_WARN, "this: %p, load ai model failed. result: %d.", this, result);
        }

        // temp load beauty image here
        result = m_pBeautyEngine->setBeautyResourcePath(config_._beautyResPath.c_str());
        if (result) {
            PRINT_LOG(SimpleLogger::LOG_TYPE::L_WARN, "this: %p, set beauty resource path failed. result: %d.", this, result);
        }

        init_ = true;
    }
  return 0;
}

int CFaceBeautyVideoFilter::stop() {
    PRINT_LOG(SimpleLogger::LOG_TYPE::L_FUNC, "%s: this: %p, init: %d", __FUNCTION__, this, init_);
    if (init_)
    {
        if (m_pBeautyEngine != nullptr) {
            int result = m_pBeautyEngine->destroyWindowsEngine();
            delete m_pBeautyEngine;
            m_pBeautyEngine = nullptr;
            init_ = false;
        }
    }
    return 0;
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

void CFaceBeautyVideoFilter::getProcessMode(ProcessMode& mode, bool& isolated) {
    mode = ProcessMode::kSync;
    isolated = true;
}

void CFaceBeautyVideoFilter::getVideoFormatWanted(agora::rtc::VideoFrameData::Type& type, agora::rtc::RawPixelBuffer::Format& format) {
    type = agora::rtc::VideoFrameData::Type::kRawPixels;
    format = agora::rtc::RawPixelBuffer::Format::kI420;
}

agora::rtc::IExtensionVideoFilter::ProcessResult CFaceBeautyVideoFilter::adaptVideoFrame(agora_refptr<rtc::IVideoFrame> in, agora_refptr<rtc::IVideoFrame>& out) {
    if (!in || m_pBeautyEngine == nullptr) {
      return kBypass;
    }

    agora::rtc::VideoFrameData frame;
    in->getVideoFrameData(frame);

    if (frame.type != agora::rtc::VideoFrameData::Type::kRawPixels || frame.pixels.format != agora::rtc::RawPixelBuffer::Format::kI420) {
      return kBypass;
    }

    int stride_uv = (frame.width + 1) / 2;
    uint8_t* data = frame.pixels.data;
    uint8_t* buffer_y = data;
    uint8_t* buffer_u = data + frame.width * frame.height;
    uint8_t* buffer_v = buffer_u + stride_uv * frame.height / 2;

    m_pBeautyEngine->processYUV(buffer_y, buffer_u, buffer_v, frame.width, frame.height, frame.rotation);
    m_pBeautyEngine->getOutputYUVData(buffer_y, buffer_u, buffer_v);
    out = in;
    return kSuccess;
}

int CFaceBeautyVideoFilter::setProperty(const char* key, const void* buf, size_t buf_size) {
    PRINT_LOG(SimpleLogger::LOG_TYPE::L_FUNC, "%s: this: %p, key: %s, buf: %s.", __FUNCTION__, this, key, (buf ? (char*)buf : "null"));
    //auto type = m_xhs_command_dict[key];
    auto it = m_xhs_command_dict.find(key);
    if (it == m_xhs_command_dict.end())
    {
        PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, invalid xhs command key: %s.", this, key);
        return -1;
    }
    auto type = it->second;
    if (m_pBeautyEngine == nullptr || buf == nullptr)
    {
        PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, m_pBeautyEngine or buf null.", this);
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
            PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, aid _subPath is empty, type: %d.", this, type);
            return -1;
        }
        std::replace(aid._subPath.begin(), aid._subPath.end(), '/', '\\');
        m_pBeautyEngine->setFilterResourcePath(aid._subPath.c_str());
        m_pBeautyEngine->setFilterIntensity(aid._lut_intensity);
    }

    case XHS_PLUGIN_COMMAND_INVALID:
        PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, invalid type: %d.",this, type);
        break;
    default:
        PRINT_LOG(SimpleLogger::LOG_TYPE::L_ERROR, "this: %p, unknown type: %d\n",this, type);
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

}
}
