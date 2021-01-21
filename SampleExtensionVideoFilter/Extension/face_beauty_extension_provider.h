#pragma once
#include "AgoraRefPtr.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionProvider.h"
#include "NGIAgoraExtensionControl.h"
#include "data_types.h"

class CFaceBeautyVideoFilterProvider : public agora::rtc::IExtensionProvider {
 public:
  CFaceBeautyVideoFilterProvider(const EngineInitParamsAid& config);
  ~CFaceBeautyVideoFilterProvider();
 
  void setExtensionControl(agora::rtc::IExtensionControl* control) override {control_ = control;}

  agora::rtc::IExtensionProvider::PROVIDER_TYPE getProviderType() override {
      return agora::rtc::IExtensionProvider::LOCAL_VIDEO_FILTER;
  }
  agora::agora_refptr<agora::rtc::IAudioFilter> createAudioFilter() override {return nullptr;};
  agora::agora_refptr<agora::rtc::IVideoFilter> createVideoFilter() override;
  agora::agora_refptr<agora::rtc::IVideoSinkBase> createVideoSink() override {return nullptr;};
 
 private:
  agora::rtc::IExtensionControl* control_;
  EngineInitParamsAid config_;
};