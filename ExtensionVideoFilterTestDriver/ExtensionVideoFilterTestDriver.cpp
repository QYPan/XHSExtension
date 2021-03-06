// ExtensionVideoFilterTestDriver.cpp : Defines the entry point for the application.
//
#include <thread>
#include <chrono>
#include "data_types.h"
#include "framework.h"
#include "ExtensionVideoFilterTestDriver.h"
#include "AgoraBase.h"
#include "IAgoraRtcEngine.h"
#include "IAgoraRtcEngineEx.h"
#include "EventHandler.h"
#include "third_party/json.hpp"

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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
    auto engine = static_cast<agora::rtc::IRtcEngineEx*>(createAgoraRtcEngine());
    if (!engine) {
        return -1;
    }

    agora::rtc::RtcEngineContextEx ctx;
    ctx.appId = "";
    ctx.enableAudio = true;
    ctx.enableVideo = false;
    ctx.extensions = new agora::rtc::Extension[1];
    ctx.extensions[0].id = "face_beauty.xhs";
    // get current exe path
    char szFullPath[MAX_PATH];
    ::GetModuleFileNameA(NULL, szFullPath, MAX_PATH);
    std::string path{ szFullPath };
    auto pos = path.find_last_of('\\');
    path = path.replace(pos, path.length() - pos, "");
    std::string dll_path = path + "/extensions/face_beauty.xhs/XHSFaceBeautyExtension.dll";
    ctx.extensions[0].path = dll_path.c_str();
    EngineInitParamsAid config;
    config._license = path + "/extensions/face_beauty.xhs";
    config._userId = "test-driver";
    config._aiModelPath = path + "/extensions/face_beauty.xhs/slim-320.face_kpt_v2.mouth.eyebrow.bin";
    config._beautyResPath = path + "/extensions/face_beauty.xhs/Beauty_Res";
    std::string config_json = config.to_json();
    ctx.extensions[0].config =  config_json.c_str();
    ctx.numExtension = 1;
    ctx.eventHandlerEx = new RtcEngineEventHandlerEx();

    // initialize
    int error = engine->initialize(ctx);

    error = engine->setChannelProfile(agora::CHANNEL_PROFILE_LIVE_BROADCASTING);

    // set canvas
    agora::rtc::uid_t uid = 0;
    /*agora::rtc::VideoCanvas vc;
    vc.uid = uid;
    //vc.view = m_videoViews[k].wndCamera.GetSafeHwnd();
    vc.renderMode = agora::media::base::RENDER_MODE_TYPE::RENDER_MODE_FIT;
    vc.mirrorMode = agora::rtc::VIDEO_MIRROR_MODE_TYPE::VIDEO_MIRROR_MODE_DISABLED;

    engine->setupLocalVideo(vc);*/

    // join channel
    agora::rtc::ChannelMediaOptions op;
    op.publishAudioTrack = false;
    op.publishCameraTrack = true;
    op.publishCustomVideoTrack = false;
    op.publishScreenTrack = false;
    op.autoSubscribeAudio = false;
    op.autoSubscribeVideo = false;
    op.clientRoleType = agora::rtc::CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER;
    error = engine->joinChannel(0, "testX", uid, op);

    if (error) {
        return error;
    }

    nlohmann::json j = true;
    error = engine->setExtensionProperty("face_beauty.xhs", "XHS_PLUGIN_BEAUTY_FILTER_SWITCH", j.dump().c_str());
    error = engine->setExtensionProperty("face_beauty.xhs", "XHS_PLUGIN_COLOR_FILTER_SWITCH", j.dump().c_str());

    float intensity = 0.25;
    while(true) {
        BeautyFilterAid aid = { FaceBeautyType::XHS_SKIN_WHITENING, true, intensity };
        engine->setExtensionProperty("face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid.to_json().c_str());

        aid = BeautyFilterAid{ FaceBeautyType::XHS_SKIN_SMOOTH, true, intensity };
        engine->setExtensionProperty("face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid.to_json().c_str());

        aid = BeautyFilterAid{ FaceBeautyType::XHS_ROUND_EYE, true, intensity };
        engine->setExtensionProperty("face_beauty.xhs", "XHS_PLUGIN_BEAUTY_TYPE", aid.to_json().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(2));

        intensity += 0.125;

        if (intensity > 1.00) {
            intensity = 0.25;
        }
    }

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
    delete[] ctx.extensions;
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
