#include "IAgoraRtcEngine.h"
#include "IAgoraRtcEngineEx.h"

class RtcEngineEventHandlerEx : public agora::rtc::IRtcEngineEventHandlerEx {
 public:
  explicit RtcEngineEventHandlerEx() {}

 private:
  void onJoinChannelSuccess(agora::rtc::conn_id_t conn_id, const char* channel, agora::rtc::uid_t uid, int elapsed) override;
  void onLeaveChannel(agora::rtc::conn_id_t conn_id, const agora::rtc::RtcStats& stats) override;
  void onExtensionEvent(const char* provider_name, const char* ext_name, const char* key, const char* json_value) override;
};