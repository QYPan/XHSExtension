#pragma once
#include <string>
#include <map>
#include "AgoraMediaBase.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionControl.h"
#include "XYCGWindowsEngine.h"
#include "../third_party/json.hpp"

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

NLOHMANN_JSON_SERIALIZE_ENUM(FaceBeautyType, {
    {XHS_SKIN_WHITENING, "skin_whitening"},
    {XHS_SKIN_SMOOTH, "skin_smooth"},
    {XHS_THIN_FACE, "thin_face"},
    {XHS_NARROW_FACE, "narrow_face"},
    {XHS_SHORT_FACE, "short_face"},
    {XHS_BIG_EYE, "big_face"},
    {XHS_THIN_JAW_BONE, "thin_jaw_bone"},
    {XHS_SHRINKAGE_PHILTRUM, "shrinking_philtrum"},
    {XHS_THIN_NOSE, "thin_nose"},
    {XHS_GROWTH_NOSE, "growth_nose"},
    {XHS_SHORT_CHIN, "short_chin"},
    {XHS_EYE_DISTANCE, "eye_distance"},
    {XHS_MOUTH_RESHAPE, "mouth_reshape"},
    {XHS_OPEN_EYE_CORNER, "open_eye_corner"},
    {XHS_ROUND_EYE, "round_eye"},
    {XHS_FOREHEAD_PLUMP, "forehead_plump"},
    {XHS_RHINOPLASTY, "thinoplasty"},
    {XHS_CHEEKBONES, "cheek_bone"},
    {XHS_SMILE_CORNER_MOUTH, "smile_corner_mouth"},
    {XHS_SPARKLE_EYE, "sparkle_eye"},
    {XHS_WHITEN_TEENTH, "white_teeth"},
    {XHS_EYEBROWS_THICK, "eyebrows_thick"},
    {XHS_EYEBROWS_POSITION, "eyebrows_position"},
    {XHS_EYE_ROTATE, "eye_rotate"},
    {XHS_LIP_RUDDY, "lip_ruddy"},
    {XHS_LOCAL_EXFOLIATING_BLACK_EYE, "local_exfoliating_black_eye"},
    {XHS_LOCAL_EXFOLIATING_NASOLABIAL_FOLDS, "local_exfoliating_nasolabial_folds"},
    })

struct BeautyFilterAid {
    FaceBeautyType  _type;
    bool            _switch = false;
    float           _beauty_type_intensity = 0.0f;

    std::string to_json() {
        nlohmann::json j = nlohmann::json{ {"type", _type}, {"switch", _switch}, {"beauty_type_intensity", _beauty_type_intensity} };
        return j.dump();
    }

    void from_json(std::string s) {
        auto j = nlohmann::json::parse(s);
        j.at("type").get_to(_type);
        j.at("switch").get_to(_switch);
        j.at("beauty_type_intensity").get_to(_beauty_type_intensity);
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM(xhs_colorful_type, {
    {XHS_COLORFUL_BRIGHT, "colorful_bright"},
    {XHS_COLORFUL_CONTRAST, "colorful_contrast"},
    {XHS_COLORFUL_WHITEBALANCE, "colorful_white_balance"},
    {XHS_COLORFUL_SATURATION, "colorful_saturation"},
    {XHS_COLORFUL_GRAININESS, "colorful_graininess"},
    })

struct ColorFilterAid {
    xhs_colorful_type   _type;
    bool                _switch = false;
    float               _color_intensity = 0.0f;

    std::string to_json() {
        nlohmann::json j = nlohmann::json{ {"type", _type}, {"switch", _switch}, {"color_intensity", _color_intensity} };
        return j.dump();
    }

    void from_json(std::string s) {
        auto j = nlohmann::json::parse(s);
        j.at("type").get_to(_type);
        j.at("switch").get_to(_switch);
        j.at("color_intensity").get_to(_color_intensity);
    }
};

struct LutFilterAid {
    std::string _subPath;
    float       _lut_intensity = 0.0f;

    std::string to_json() {
        nlohmann::json j = nlohmann::json{ {"sub_path", _subPath}, {"lut_intensity", _lut_intensity} };
        return j.dump();
    }

    void from_json(std::string s) {
        auto j = nlohmann::json::parse(s);
        j.at("sub_path").get_to(_subPath);
        j.at("lut_intensity").get_to(_lut_intensity);
    }
};


class CSampleVideoFilter : public agora::rtc::IVideoFilter {
 public:
  CSampleVideoFilter(const char* id, agora::rtc::IExtensionControl* core);
  ~CSampleVideoFilter();

  bool onDataStreamWillStart() override;

  void onDataStreamWillStop() override;

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