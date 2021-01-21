#include "pch.h"
#define EXPORT_DLL 1
#include "AgoraRefCountedObject.h"
#include "exported_functions.h"
#include "Extension/face_beauty_extension_provider.h"
#include "Extension/data_types.h"

namespace vendor_xhs {
agora::agora_refptr<agora::rtc::IExtensionProvider> __stdcall GetExtensionProvider(const char* engineInitParamsJson) {
    EngineInitParamsAid aid;
    if (engineInitParamsJson && *engineInitParamsJson) {
        aid.from_json(engineInitParamsJson);
    }

    return new agora::RefCountedObject<CFaceBeautyVideoFilterProvider>(aid);
}
}

#undef EXPORT_DLL