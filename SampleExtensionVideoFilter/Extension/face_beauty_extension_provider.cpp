#include "pch.h"
#include "AgoraRefCountedObject.h"
#include "face_beauty_extension_provider.h"
#include "face_beauty_video_filter.h"

namespace agora {
namespace extension {

CFaceBeautyVideoFilterProvider::CFaceBeautyVideoFilterProvider() {
}

CFaceBeautyVideoFilterProvider::~CFaceBeautyVideoFilterProvider() {
}

agora::agora_refptr<agora::rtc::IExtensionVideoFilter> CFaceBeautyVideoFilterProvider::createVideoFilter(const char* name) {
  return new agora::RefCountedObject<CFaceBeautyVideoFilter>("face_beauty.xhs", config_, control_);
}

void CFaceBeautyVideoFilterProvider::enumerateExtensions(ExtensionMetaInfo* extension_list, int& extension_count) {
  extension_count = 1;
  ExtensionMetaInfo i;
  i.type = EXTENSION_TYPE::VIDEO_PRE_PROCESSING_FILTER;
  i.extension_name = "face_beauty.xhs";
  extension_list[0] = i;
}

void CFaceBeautyVideoFilterProvider::setProperty(const char* key, const char* value) {
  if (value && *value) {
	EngineInitParamsAid aid;
    aid.from_json(value);
	config_ = aid;
	std::replace(config_._license.begin(), config_._license.end(), '/', '\\');
	std::replace(config_._aiModelPath.begin(), config_._aiModelPath.end(), '/', '\\');
	std::replace(config_._beautyResPath.begin(), config_._beautyResPath.end(), '/', '\\');
  }
}

}
}