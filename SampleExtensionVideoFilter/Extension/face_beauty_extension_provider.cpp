#include "pch.h"
#include "AgoraRefCountedObject.h"
#include "face_beauty_extension_provider.h"
#include "face_beauty_video_filter.h"

CFaceBeautyVideoFilterProvider::CFaceBeautyVideoFilterProvider(const EngineInitParamsAid& config) {
}

CFaceBeautyVideoFilterProvider::~CFaceBeautyVideoFilterProvider() {
}

agora::agora_refptr<agora::rtc::IVideoFilter> CFaceBeautyVideoFilterProvider::createVideoFilter() {
  return new agora::RefCountedObject<CFaceBeautyVideoFilter>("test", control_);
}