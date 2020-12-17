#include "pch.h"
#include "AgoraRefCountedObject.h"
#include "sample_video_filter_provider.h"
#include "sample_video_filter_extension.h"

CSampleVideoFilterProvider::CSampleVideoFilterProvider() {
}

CSampleVideoFilterProvider::~CSampleVideoFilterProvider() {
}

agora::agora_refptr<agora::rtc::IVideoFilter> CSampleVideoFilterProvider::createVideoFilter(const char* id) {
  return new agora::RefCountedObject<CSampleVideoFilter>(id, control_);
}