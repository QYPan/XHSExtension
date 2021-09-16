#include "./EventHandler.h"

void RtcEngineEventHandlerEx::onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed) {
    return;
}

void RtcEngineEventHandlerEx::onLeaveChannel(const agora::rtc::RtcStats& stats) {
    return;
}

void RtcEngineEventHandlerEx::onExtensionEvent(const char* provider_name, const char* ext_name, const char* key, const char* json_value) {
    (void)provider_name;
    (void)ext_name;
    (void)key;
    (void)json_value;
}