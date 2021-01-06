#pragma once
#include <string>
#include "AgoraMediaBase.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionControl.h"
#include "XYCGWindowsEngine.h"
#include <map>

typedef enum {
    XHS_PLUGIN_COMMAND_INVALID,
    XHS_PLUGIN_BEAUTY_FILTER_SWITCH,       /// beauty filter switch. buf should be a bool
    XHS_PLUGIN_BEAUTY_TYPE,                /// beauty type filter params set. buf should be BeautyFilterAid
    XHS_PLUGIN_COLOR_FILTER_SWITCH,        /// color filter switch. buf should be a bool
    XHS_PLUGIN_COLOR_TYPE,                 /// color type filter params set. buf should be ColorFilterAid
    XHS_PLUGIN_LUT_FILTER_SWITCH,          /// lut filter switch. buf should be a bool
    XHS_PLUGIN_LUT_FILTER_TYPE,            /// lut filter params set, include path of lut&json files. buf should be LutFilterAid
    XHS_PLUGIN_COMMAND_COUNT
} xhs_Command_type;

xhs_Command_type getKeyCommandType(const char* key);

struct BeautyFilterAid {
    FaceBeautyType  _type;
    bool            _switch = false;
    float           _beauty_type_intensity = 0.0f;
};

struct ColorFilterAid {
    xhs_colorful_type   _type;
    bool                _switch = false;
    float               _color_intensity = 0.0f;
};

struct LutFilterAid {
    char*       _subPath = nullptr;
    float       _lut_intensity = 0.0f;
};


class CSampleVideoFilter : public agora::rtc::IVideoFilter {
 public:
  CSampleVideoFilter(const char* id, agora::rtc::IExtensionControl* core);
  ~CSampleVideoFilter();

  void setEnabled(bool enable) override {
    enabled_ = enable;
  }

  bool isEnabled() override {
    return enabled_;
  }

  size_t setProperty(const char* key, const void* buf, size_t buf_size) override;

  bool adaptVideoFrame(const agora::media::base::VideoFrame& capturedFrame,
                               agora::media::base::VideoFrame& adaptedFrame) override;



 private:
  bool enabled_;
  std::string id_;
  agora::rtc::IExtensionControl* core_;
  CG::XYCGWindowsEngine* m_pBeautyEngine;
  bool init_;
  int frameCount_ = 0;

  
};