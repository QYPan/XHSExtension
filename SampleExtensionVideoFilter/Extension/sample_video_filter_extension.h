#pragma once
#include <string>
#include "AgoraMediaBase.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionControl.h"

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
};