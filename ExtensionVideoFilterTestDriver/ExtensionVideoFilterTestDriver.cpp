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

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

std::string GetAppId() {
    std::ifstream in("C:/Users/qyou/space/appid.txt");
    std::string appid;
    in >> appid;
    in.close();
    return appid;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::string provider_name("xiaohongshu");

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EXTENSIONVIDEOFILTERTESTDRIVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EXTENSIONVIDEOFILTERTESTDRIVER));

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
    auto pos = path.find_last_of('\\');
    path = path.replace(pos, path.length() - pos, "");
    std::string dll_path = path + "\\extensions\\face_beauty.xhs\\XHSFaceBeautyExtension.dll";

    int ret = engine->loadExtensionProvider(dll_path.c_str());

    char logret[100] = {0};
    sprintf(logret, "\nload ret: %d\n", ret);
    OutputDebugStringA(logret);

    ret = engine->enableExtension(provider_name.c_str(), "face_beauty.xhs", true, agora::media::PRIMARY_CAMERA_SOURCE);
    sprintf(logret, "\nenableExtension ret1: %d\n", ret);
    OutputDebugStringA(logret);

    ret = engine->enableExtension(provider_name.c_str(), "face_beauty.xhs", true, agora::media::SECONDARY_CAMERA_SOURCE);
    sprintf(logret, "\nenableExtension ret2: %d\n", ret);
    OutputDebugStringA(logret);

    EngineInitParamsAid config;
    config._license = path + "/extensions/face_beauty.xhs";
    config._userId = "test-driver";
    config._aiModelPath = path + "/extensions/face_beauty.xhs/slim-320.face_kpt_v2.mouth.eyebrow.bin";
    config._beautyResPath = path + "/extensions/face_beauty.xhs/Beauty_Res";
    std::string config_json = config.to_json();

    ret = engine->setExtensionProviderProperty(provider_name.c_str(), "", config_json.c_str());
    sprintf(logret, "\nsetExtensionProperty config, ret: %d\n", ret);
    OutputDebugStringA(logret);

    error = engine->setChannelProfile(agora::CHANNEL_PROFILE_LIVE_BROADCASTING);

    // set canvas
    agora::rtc::uid_t uid = 0;
    /*agora::rtc::VideoCanvas vc;
    vc.uid = uid;
    //vc.view = m_videoViews[k].wndCamera.GetSafeHwnd();
    vc.renderMode = agora::media::base::RENDER_MODE_TYPE::RENDER_MODE_FIT;
    vc.mirrorMode = agora::rtc::VIDEO_MIRROR_MODE_TYPE::VIDEO_MIRROR_MODE_DISABLED;

    engine->setupLocalVideo(vc);*/

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

    // 获取摄像头设备
    for (int idx = 0; idx < dev_cnt && idx < 2; idx++) {
      vdc->getDevice(idx, dev_names[idx], camera_config[idx].deviceId);
      //AGO_LOG("Get video device, idx: %d, dev_name: %s, dev_id: %s.", idx, dev_names[idx], camera_config[idx].deviceId);
    }

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

    // 加入频道
    agora::rtc::ChannelMediaOptions op;
    op.publishTrancodedVideoTrack = true;
    op.clientRoleType = agora::rtc::CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER;
    error = engine->joinChannel(0, "xhs123", uid, op);

    if (error) {
        return error;
    }

    //Sleep(3000);

    //system("pause");

    // 开启第一第二路美颜开关
    //******************************************************************************************************************************************************************
    nlohmann::json j = true;
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_FILTER_SWITCH", j.dump().c_str(), agora::media::PRIMARY_CAMERA_SOURCE);
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_COLOR_FILTER_SWITCH", j.dump().c_str(), agora::media::PRIMARY_CAMERA_SOURCE);

    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_FILTER_SWITCH", j.dump().c_str(), agora::media::SECONDARY_CAMERA_SOURCE);
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_COLOR_FILTER_SWITCH", j.dump().c_str(), agora::media::SECONDARY_CAMERA_SOURCE);

    //******************************************************************************************************************************************************************

    // 设置第一路美颜参数
    BeautyFilterAid aid1 = { FaceBeautyType::XHS_NARROW_FACE, true, 41.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid1.to_json().c_str(), agora::media::PRIMARY_CAMERA_SOURCE);
    //system("pause");

    // 设置第二路美颜参数
    BeautyFilterAid aid2 = { FaceBeautyType::XHS_NARROW_FACE, true, 13.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid2.to_json().c_str(), agora::media::SECONDARY_CAMERA_SOURCE);
    system("pause");

    //******************************************************************************************************************************************************************

    BeautyFilterAid aid3 = { FaceBeautyType::XHS_NARROW_FACE, true, 454.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid3.to_json().c_str(), agora::media::PRIMARY_CAMERA_SOURCE);
    //system("pause");

    BeautyFilterAid aid4 = { FaceBeautyType::XHS_NARROW_FACE, true, 5.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid4.to_json().c_str(), agora::media::SECONDARY_CAMERA_SOURCE);
    system("pause");

    //******************************************************************************************************************************************************************

    BeautyFilterAid aid5 = { FaceBeautyType::XHS_NARROW_FACE, true, 111 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid5.to_json().c_str(), agora::media::PRIMARY_CAMERA_SOURCE);
    //system("pause");

    BeautyFilterAid aid6 = { FaceBeautyType::XHS_NARROW_FACE, true, 51.882 };
    engine->setExtensionProperty(provider_name.c_str(), "face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid6.to_json().c_str(), agora::media::SECONDARY_CAMERA_SOURCE);
    system("pause");

    std::this_thread::sleep_for(std::chrono::seconds(180));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    engine->release();
    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXTENSIONVIDEOFILTERTESTDRIVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EXTENSIONVIDEOFILTERTESTDRIVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
