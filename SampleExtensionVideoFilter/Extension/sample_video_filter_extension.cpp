#include "pch.h"
#include "sample_video_filter_extension.h"

CSampleVideoFilter::CSampleVideoFilter(const char* id, agora::rtc::IExtensionControl* core)
  : enabled_(true), id_(id), core_(core) {
}

CSampleVideoFilter::~CSampleVideoFilter() {
}

bool CSampleVideoFilter::adaptVideoFrame(const agora::media::base::VideoFrame& capturedFrame,
                               agora::media::base::VideoFrame& adaptedFrame) {
    adaptedFrame = capturedFrame;
    if (core_) {
      core_->log(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO, "accept one video frame");
      core_->fireEvent(id_.c_str(), "test_key", "test_value");
    }
    return true;
}

size_t CSampleVideoFilter::setProperty(const char* key, const void* buf, size_t buf_size) {
  return -1;
}