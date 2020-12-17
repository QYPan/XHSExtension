#pragma once
#include "AgoraRefPtr.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionProvider.h"
#include "NGIAgoraExtensionControl.h"

class CSampleVideoFilterProvider : public agora::rtc::IExtensionProvider {
 public:
  CSampleVideoFilterProvider();
  ~CSampleVideoFilterProvider();
 
  void setExtensionControl(agora::rtc::IExtensionControl* control) override {control_ = control;}

  agora::rtc::IExtensionProvider::PROVIDER_TYPE getProviderType() override {
      return agora::rtc::IExtensionProvider::LOCAL_VIDEO_FILTER;
  }
  agora::agora_refptr<agora::rtc::IAudioFilter> createAudioFilter(const char* id) override {return nullptr;};
  agora::agora_refptr<agora::rtc::IVideoFilter> createVideoFilter(const char* id) override;
  agora::agora_refptr<agora::rtc::IVideoSinkBase> createVideoSink(const char* id) override {return nullptr;};
 
 private:
  agora::rtc::IExtensionControl* control_;
};