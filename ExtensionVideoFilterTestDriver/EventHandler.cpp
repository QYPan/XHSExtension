#include "./EventHandler.h"

void RtcEngineEventHandlerEx::onJoinChannelSuccess(agora::rtc::conn_id_t conn_id, const char* channel, agora::rtc::uid_t uid, int elapsed) {
    return;
}

void RtcEngineEventHandlerEx::onLeaveChannel(agora::rtc::conn_id_t conn_id, const agora::rtc::RtcStats& stats) {
    return;
}

void RtcEngineEventHandlerEx::onExtensionEvent(const char* id, const char* key, const char* json_value) {
    (void)id;
    (void)key;
    (void)json_value;
}