#pragma once
#include "AgoraRefPtr.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraExtensionProvider.h"
#include "NGIAgoraExtensionControl.h"
#include "data_types.h"

namespace agora {
namespace extension {

class CFaceBeautyVideoFilterProvider : public agora::rtc::IExtensionProvider {
 public:
  CFaceBeautyVideoFilterProvider();
  ~CFaceBeautyVideoFilterProvider();
 
  void setExtensionControl(agora::rtc::IExtensionControl* control) override {control_ = control;}
  agora::agora_refptr<agora::rtc::IAudioFilter> createAudioFilter(const char* name) override {return nullptr;};
  agora::agora_refptr<agora::rtc::IExtensionVideoFilter> createVideoFilter(const char* name) override;
  agora::agora_refptr<agora::rtc::IVideoSinkBase> createVideoSink(const char* name) override {return nullptr;};
  void enumerateExtensions(ExtensionMetaInfo* extension_list, int& extension_count) override;
  void setProperty(const char* key, const char* value) override;

 private:
  agora::rtc::IExtensionControl* control_;
  EngineInitParamsAid config_;
};

}
}