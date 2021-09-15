#pragma once
#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include <windows.h>

class SimpleWindow {
 public:
  explicit SimpleWindow(std::string title);
  ~SimpleWindow();

  HWND GetView();

 private:
  static LRESULT WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  HWND hwnd_;
  std::unique_ptr<std::thread> looper_;
  std::atomic<bool> stop_;
};
