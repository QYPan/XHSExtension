#include "pch.h"
#define EXPORT_DLL 1
#include "AgoraRefCountedObject.h"
#include "exported_functions.h"
#include "../Extension/sample_video_filter_provider.h"

namespace sample_vendor {
agora::agora_refptr<agora::rtc::IExtensionProvider> __stdcall GetExtensionProvider(const void* engineInitParamsJson) {
    //setEngineInitParams aid;
    //aid.fromJson(engineInitParamsJson);
    //void CSampleVideoFilter::setResPath(aid) // call this fun here or global variable?
    return new agora::RefCountedObject<CSampleVideoFilterProvider>();
}
}

#undef EXPORT_DLL