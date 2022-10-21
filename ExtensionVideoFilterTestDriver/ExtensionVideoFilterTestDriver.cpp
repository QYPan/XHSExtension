// ExtensionVideoFilterTestDriver.cpp : Defines the entry point for the application.
//
#include <thread>
#include <fstream>
#include <chrono>
#include "data_types.h"
#include "framework.h"
#include "ExtensionVideoFilterTestDriver.h"
#include "AgoraBase.h"
#include "IAgoraRtcEngine.h"
#include "IAgoraRtcEngineEx.h"
#include "EventHandler.h"
#include "third_party/json.hpp"

#include "SimpleWindow.h"

std::string GetAppId() {
    std::ifstream in("C:/Users/qyou/space/appid.txt");
    std::string appid;
    in >> appid;
    in.close();
    return appid;
}

std::string ansi2utf8(const std::string& ansi) {
  if (ansi.empty()) return std::string();
  int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), nullptr, 0);
  wchar_t* widestr = new wchar_t[len + 1];
  if (!widestr) return std::string();
  ZeroMemory(widestr, sizeof(wchar_t) * (len + 1));
  MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), widestr, len + 1);

  int utf8len = WideCharToMultiByte(CP_UTF8, 0, widestr, len + 1, nullptr, 0, nullptr, nullptr);

  char* utf8str = new char[utf8len + 1];
  if (!utf8str) {
    delete[] widestr;
    return std::string();
  }
  ZeroMemory(utf8str, utf8len + 1);
  WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)widestr, len + 1, utf8str, utf8len + 1, nullptr, nullptr);
  std::string result = utf8str;
  delete[] widestr;
  delete[] utf8str;
  return result;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    std::string provider_name("xiaohongshu");

    // initialize rtc engine
    auto engine = static_cast<agora::rtc::IRtcEngine*>(createAgoraRtcEngine());
    if (!engine) {
        return -1;
    }

    agora::rtc::RtcEngineContext ctx;
    std::string appid = GetAppId();
    ctx.appId = appid.c_str();
    ctx.eventHandler = new RtcEngineEventHandlerEx();
    // initialize
    int error = engine->initialize(ctx);

    // get current exe path
    char szFullPath[MAX_PATH];
    ::GetModuleFileNameA(NULL, szFullPath, MAX_PATH);
    std::string path{ szFullPath };
    path = ansi2utf8(path);
    auto pos = path.find_last_of('\\');
    path = path.replace(pos, path.length() - pos, "");
    std::string dll_path = path + "\\extensions\\face_beauty.xhs\\XHSFaceBeautyExtension.dll";

    int ret = engine->loadExtensionProvider(dll_path.c_str());

    char logret[100] = {0};
    sprintf(logret, "\nload ret: %d\n", ret);
    OutputDebugStringA(logret);

    EngineInitParamsAid config;
    config._license = path + "/extensions/face_beauty.xhs";
    config._userId = "test-driver";
    config._aiModelPath = path + "/extensions/face_beauty.xhs/slim-320.face_kpt_v2.mouth.eyebrow.bin";
    config._beautyResPath = path + "/extensions/face_beauty.xhs/Beauty_Res";
    std::string config_json = config.to_json();

    // 存储用户美颜库初始所需信息到 provider，该信息会在 video filter start() 时用到
    ret = engine->setExtensionProviderProperty(provider_name.c_str(), "", config_json.c_str());
    sprintf(logret, "\nsetExtensionProperty config, ret: %d\n", ret);
    OutputDebugStringA(logret);

    ret = engine->enableExtension(provider_name.c_str(), "face_beauty.xhs", true, agora::media::PRIMARY_CAMERA_SOURCE);
    sprintf(logret, "\nenableExtension ret1: %d\n", ret);
    OutputDebugStringA(logret);

    ret = engine->enableExtension(provider_name.c_str(), "face_beauty.xhs", true, agora::media::SECONDARY_CAMERA_SOURCE);
    sprintf(logret, "\nenableExtension ret2: %d\n", ret);
    OutputDebugStringA(logret);

    agora::rtc::VideoEncoderConfiguration cfg;
    cfg.codecType = agora::rtc::VIDEO_CODEC_H264;
    cfg.dimensions = {1280, 720};
    cfg.bitrate = 2000;
    cfg.frameRate = 30;
    engine->setVideoEncoderConfiguration(cfg);

    agora::util::AutoPtr<agora::rtc::IVideoDeviceManager> vdm;
    vdm.queryInterface(engine, agora::rtc::AGORA_IID_VIDEO_DEVICE_MANAGER);
    agora::util::AutoPtr<agora::rtc::IVideoDeviceCollection> vdc(vdm->enumerateVideoDevices());

    agora::rtc::CameraCapturerConfiguration camera_config[2];
    char dev_names[2][agora::rtc::MAX_DEVICE_ID_LENGTH] = {0};
    int dev_cnt = vdc->getCount();

    vdc->getDevice(0, dev_names[0], camera_config[0].deviceId);
    vdc->getDevice(4, dev_names[1], camera_config[1].deviceId);

    camera_config[0].format.width = 1280;
    camera_config[0].format.height = 720;
    camera_config[0].format.fps = 20;

    // 开启第一路摄像头采集
    engine->startPrimaryCameraCapture(camera_config[0]);

    if (dev_cnt > 1) {
      camera_config[1].format.width = 1280;
      camera_config[1].format.height = 720;
      camera_config[1].format.fps = 20;

      // 开启第二路摄像头采集
      engine->startSecondaryCameraCapture(camera_config[1]);
    }

    agora::rtc::TranscodingVideoStream stream_infos[2];

    stream_infos[0].sourceType = agora::media::PRIMARY_CAMERA_SOURCE;
    stream_infos[0].x = 0;
    stream_infos[0].y = 0;
    stream_infos[0].width = 640;
    stream_infos[0].height = 360;
    stream_infos[0].mirror = false;
    stream_infos[0].zOrder = 2;

    if (dev_cnt > 1) {
      stream_infos[1].sourceType = agora::media::SECONDARY_CAMERA_SOURCE;
      stream_infos[1].x = 640;
      stream_infos[1].y = 0;
      stream_infos[1].width = 640;
      stream_infos[1].height = 360;
      stream_infos[1].mirror = false;
      stream_infos[1].zOrder = 3;
    }

    agora::rtc::VideoEncoderConfiguration encoder_config;
    encoder_config.codecType = agora::rtc::VIDEO_CODEC_H264;
    encoder_config.dimensions = {1280, 720};
    encoder_config.bitrate = 2000;
    encoder_config.frameRate = 20;

    agora::rtc::LocalTranscoderConfiguration transcoder_config;
    transcoder_config.streamCount = dev_cnt > 1 ? 2 : 1;
    transcoder_config.VideoInputStreams = stream_infos;
    transcoder_config.videoOutputConfiguration = encoder_config;

    // 开启合图
    engine->startLocalVideoTranscoder(transcoder_config);

    auto transcoded_win = std::make_unique<SimpleWindow>("TranscodedWin");
    agora::rtc::VideoCanvas cv_transcoded;

    cv_transcoded.sourceType = agora::rtc::VIDEO_SOURCE_TRANSCODED;
    cv_transcoded.view = transcoded_win->GetView();
    engine->setupLocalVideo(cv_transcoded);
    engine->startPreview();

    agora::rtc::uid_t uid = 0;

    // 加入频道
    agora::rtc::ChannelMediaOptions op;
    op.publishTrancodedVideoTrack = true;
    op.clientRoleType = agora::rtc::CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER;
    error = engine->joinChannel(0, "xhs123", uid, op);

    if (error) {
        return error;
    }

    //Sleep(3000);

    //int RtcEngine::setExtensionProperty(const char* provider, const char* extension,
    //                                const ExtensionInfo& extensionInfo, const char* key,
    //                                const char* value)

    //system("pause");

    agora::rtc::ExtensionInfo primary_ext_info, secondary_ext_info;
    primary_ext_info.mediaSourceType = agora::media::PRIMARY_CAMERA_SOURCE;
    secondary_ext_info.mediaSourceType = agora::media::SECONDARY_CAMERA_SOURCE;

    // 开启第一第二路美颜开关
    //******************************************************************************************************************************************************************
    nlohmann::json j = true;
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", primary_ext_info, "XHS_PLUGIN_BEAUTY_FILTER_SWITCH", j.dump().c_str());
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", primary_ext_info, "XHS_PLUGIN_COLOR_FILTER_SWITCH", j.dump().c_str());

    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", secondary_ext_info, "XHS_PLUGIN_BEAUTY_FILTER_SWITCH", j.dump().c_str());
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", secondary_ext_info, "XHS_PLUGIN_COLOR_FILTER_SWITCH", j.dump().c_str());

    //******************************************************************************************************************************************************************

    // 设置第一路美颜参数
    BeautyFilterAid aid1 = { FaceBeautyType::XHS_NARROW_FACE, true, 41.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", primary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid1.to_json().c_str());
    //system("pause");

    // 设置第二路美颜参数
    BeautyFilterAid aid2 = { FaceBeautyType::XHS_NARROW_FACE, true, 13.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", secondary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid2.to_json().c_str());
    system("pause");

    //******************************************************************************************************************************************************************

    BeautyFilterAid aid3 = { FaceBeautyType::XHS_NARROW_FACE, true, 454.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", primary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid3.to_json().c_str());
    //system("pause");

    BeautyFilterAid aid4 = { FaceBeautyType::XHS_NARROW_FACE, true, 5.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", secondary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid4.to_json().c_str());
    system("pause");

    //******************************************************************************************************************************************************************

    BeautyFilterAid aid5 = { FaceBeautyType::XHS_NARROW_FACE, true, 111 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", primary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid5.to_json().c_str());
    //system("pause");

    BeautyFilterAid aid6 = { FaceBeautyType::XHS_NARROW_FACE, true, 51.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", secondary_ext_info, "XHS_PLUGIN_BEAUTY_TYPE", aid6.to_json().c_str());
    system("pause");

    engine->leaveChannel();
    engine->stopLocalVideoTranscoder();
    engine->stopPrimaryCameraCapture();
    if (dev_cnt > 1) {
      engine->stopSecondaryCameraCapture();
    }

    engine->release();
    return 0;
}
