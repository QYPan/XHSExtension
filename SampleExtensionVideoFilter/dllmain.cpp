// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "AgoraExtensionProviderEntry.h"
#include "Extension/face_beauty_extension_provider.h"

REGISTER_AGORA_EXTENSION_PROVIDER(xiaohongshu, agora::extension::CFaceBeautyVideoFilterProvider);