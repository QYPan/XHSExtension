#pragma once
#include "AgoraRefPtr.h"
#include "NGIAgoraExtensionProvider.h"

#ifdef EXPORT_DLL
  #define EXTENSION_API __declspec(dllexport)
#else
  #define EXTENSION_API __declspec(dllimport)
#endif

namespace sample_vendor {
  typedef agora::agora_refptr<agora::rtc::IExtensionProvider>(__stdcall* get_provider_func)();
  agora::agora_refptr<agora::rtc::IExtensionProvider> __stdcall GetExtensionProvider();
}