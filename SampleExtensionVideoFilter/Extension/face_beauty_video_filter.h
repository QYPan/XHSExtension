#pragma once
#include <map>
#include "AgoraMediaBase.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionControl.h"
#include "data_types.h"

class CFaceBeautyVideoFilter : public agora::rtc::IVideoFilter {
 public:
  CFaceBeautyVideoFilter(const char* id, agora::rtc::IExtensionControl* core);
  ~CFaceBeautyVideoFilter();

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

  void setEngineInitParams(const EngineInitParamsAid& aid);

 private:
  bool enabled_;
  std::string id_;
  agora::rtc::IExtensionControl* core_;
  bool init_;
  int frameCount_ = 0;

  CG::XYCGWindowsEngine* m_pBeautyEngine = nullptr;
  std::string m_license = "";
  std::string m_userId = "";
  std::string m_aiModelPath = "slim-320.face_kpt_v2.mouth.eyebrow.bin";
  std::string m_beautyResPath = "Beauty_Res";

};