#include "pch.h"
#include "AgoraRefCountedObject.h"
#include "face_beauty_extension_provider.h"
#include "face_beauty_video_filter.h"

CFaceBeautyVideoFilterProvider::CFaceBeautyVideoFilterProvider(const EngineInitParamsAid& config)
: config_(config) {
	std::replace(config_._license.begin(), config_._license.end(), '/', '\\');
	std::replace(config_._aiModelPath.begin(), config_._aiModelPath.end(), '/', '\\');
	std::replace(config_._beautyResPath.begin(), config_._beautyResPath.end(), '/', '\\');
}

CFaceBeautyVideoFilterProvider::~CFaceBeautyVideoFilterProvider() {
}

agora::agora_refptr<agora::rtc::IVideoFilter> CFaceBeautyVideoFilterProvider::createVideoFilter() {
  return new agora::RefCountedObject<CFaceBeautyVideoFilter>("face_beauty.xhs", config_, control_);
}