#pragma once
#include <map>
#include "AgoraMediaBase.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionControl.h"
#include "data_types.h"

namespace agora {
namespace extension {

class CFaceBeautyVideoFilter : public agora::rtc::IExtensionVideoFilter {
 public:
  CFaceBeautyVideoFilter(const char* id, const EngineInitParamsAid& config, agora::rtc::IExtensionControl* core);
  ~CFaceBeautyVideoFilter();

  int setProperty(const char *key, const void *buf, size_t buf_size) override;

  int getProperty(const char *key, void *buf, size_t buf_size) override { return 0; }

  void getProcessMode(ProcessMode& mode, bool& isolated) override;

  int start(agora::agora_refptr<Control> control) override;

  int stop() override;

  void getVideoFormatWanted(agora::rtc::VideoFrameData::Type& type, agora::rtc::RawPixelBuffer::Format& format) override;

  ProcessResult pendVideoFrame(agora::agora_refptr<rtc::IVideoFrame> frame) override { return kBypass; }

  ProcessResult adaptVideoFrame(agora_refptr<rtc::IVideoFrame> in, agora_refptr<rtc::IVideoFrame>& out) override;

 private:
  std::string id_;
  agora::rtc::IExtensionControl* core_;
  bool init_;

  CG::XYCGWindowsEngine* m_pBeautyEngine = nullptr;
  const EngineInitParamsAid config_;
};

}
}
