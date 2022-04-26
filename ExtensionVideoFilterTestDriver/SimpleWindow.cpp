#include "SimpleWindow.h"

const char kSimpleWindowClass[] = "AEC858EC-E51C-4211-A464-CE538AA27AA6";

SimpleWindow::SimpleWindow(std::string window_title, int width, int height) : hwnd_(nullptr), stop_(false) {
  static ATOM wc_atom = 0;
  if (wc_atom == 0) {
    WNDCLASSA wc = {};

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    wc.lpszClassName = kSimpleWindowClass;

    wc_atom = RegisterClassA(&wc);
    if (wc_atom == 0) return;
  }

  HANDLE created = CreateEvent(NULL, FALSE, FALSE, NULL);

  looper_ = std::unique_ptr<std::thread>(new std::thread([this, window_title, width, height, &created] {
    hwnd_ = CreateWindowA(kSimpleWindowClass, window_title.c_str(), WS_OVERLAPPEDWINDOW, 0, 0,
                          width, height, NULL, NULL, NULL, NULL);
    SetEvent(created);

    if (!hwnd_) {
      return;
    }
    // looper has to has the same thread as CreateWindow

    MSG msg;
    BOOL ret;
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
      if (stop_) {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    CloseWindow(hwnd_);
    DestroyWindow(hwnd_);
  }));

  DWORD dwResult = WaitForSingleObject(created, -1);
  if (dwResult != WAIT_OBJECT_0) {
    stop_ = true;
    return;
  }

  if (hwnd_ == NULL) {
    stop_ = true;
    return;
  }

  CloseHandle(created);

  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

SimpleWindow::~SimpleWindow() {
  if (hwnd_ != NULL) {
    stop_ = true;
    PostMessage(hwnd_, WM_PAINT, 0, 0);
    if (looper_ && looper_->joinable()) {
      looper_->join();
      looper_.reset();
    }
    hwnd_ = NULL;
  }
}

HWND SimpleWindow::GetView() { return hwnd_; }

LRESULT WINAPI SimpleWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_DESTROY || (msg == WM_CHAR && wparam == VK_RETURN)) {
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProcA(hwnd, msg, wparam, lparam);
}
