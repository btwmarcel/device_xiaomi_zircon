/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "UdfpsHandler.zircon"

#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>
#include <android-base/logging.h>
#include <android-base/unique_fd.h>

#include <bitset>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <poll.h>
#include <sys/ioctl.h>
#include <thread>

#include "UdfpsHandler.h"

#include <drm/mi_disp.h>

constexpr const char *kDispFeaturePath = "/dev/disp_feature";

constexpr int kCommandNit = 10;

constexpr int kParamNitFod = 1;
constexpr int kParamNitNone = 0;

constexpr int kCommandFodPressStatus = 1;
constexpr int kParamFodPressed = 1;
constexpr int kParamFodReleased = 0;

using ::aidl::android::hardware::biometrics::fingerprint::AcquiredInfo;

namespace {

static std::shared_ptr<disp_event_resp> parse_disp_event(int fd) {
  disp_event header;
  ssize_t header_size = read(fd, &header, sizeof(header));
  if (header_size < sizeof(header)) {
    LOG(ERROR) << "unexpected display event header size: " << header_size;
    return nullptr;
  }

  if (header.length < sizeof(disp_event)) {
    LOG(ERROR) << "invalid event length: " << header.length;
    return nullptr;
  }
  size_t data_length = header.length - sizeof(disp_event);

  std::shared_ptr<disp_event_resp> response(
      static_cast<disp_event_resp *>(malloc(header.length)), free);
  if (!response) {
    LOG(ERROR) << "failed to allocate memory for display event response";
    return nullptr;
  }
  response->base = header;

  ssize_t data_size = read(fd, &response->data, data_length);
  if (data_size < (ssize_t)data_length) {
    LOG(ERROR) << "unexpected display event data size: " << data_size;
    return nullptr;
  }

  return response;
}

struct disp_base display_base_primary = {
    .flag = 0,
    .disp_id = MI_DISP_PRIMARY,
};

} // namespace

class ZirconUdfpsHandler : public UdfpsHandler {
public:
  void init(fingerprint_device_t *device) {
    device_ = device;
    disp_fd_ = android::base::unique_fd(open(kDispFeaturePath, O_RDWR));
    if (disp_fd_ < 0) {
      LOG(ERROR) << "failed to open " << kDispFeaturePath;
    }

    std::thread([this]() {
      android::base::unique_fd fd(open(kDispFeaturePath, O_RDWR));
      if (fd < 0) {
        LOG(ERROR) << "failed to open " << kDispFeaturePath
                   << " , err: " << strerror(errno);
        return;
      }

      struct disp_event_req display_event_req = {
          .base = display_base_primary,
          .type = MI_DISP_EVENT_FOD,
      };

      if (ioctl(fd.get(), MI_DISP_IOCTL_REGISTER_EVENT, &display_event_req) <
          0) {
        LOG(ERROR) << "failed to register FOD event";
        return;
      }

      struct pollfd disp_event_poll = {
          .fd = fd.get(),
          .events = POLLIN,
          .revents = 0,
      };

      int consecutive_errors = 0;
      while (true) {
        disp_event_poll.revents = 0;
        int rc = poll(&disp_event_poll, 1, -1);
        if (rc < 0) {
          if (errno == EINTR)
            continue;
          LOG(ERROR) << "failed to poll " << kDispFeaturePath << ": "
                     << strerror(errno);
          return;
        }

        if (disp_event_poll.revents & (POLLERR | POLLHUP | POLLNVAL))
          return;

        if (!(disp_event_poll.revents & POLLIN)) {
          continue;
        }

        std::shared_ptr<disp_event_resp> response = parse_disp_event(fd.get());
        if (!response) {
          if (++consecutive_errors >= 10) {
            LOG(ERROR) << "giving up on FOD events after " << consecutive_errors
                       << " bad reads";
            return;
          }
          continue;
        }
        consecutive_errors = 0;

        if (response->base.type != MI_DISP_EVENT_FOD) {
          LOG(ERROR) << "unexpected display event: " << response->base.type;
          continue;
        }

        int value = response->data[0];
        LOG(DEBUG) << "received data: " << std::bitset<8>(value);

        bool local_hbm_ui_ready = value & LOCAL_HBM_UI_READY;

        device_->extCmd(device_, kCommandNit,
                        local_hbm_ui_ready ? kParamNitFod : kParamNitNone);
      }
    }).detach();
  }

  void onFingerDown(uint32_t _x, uint32_t _y, float _minor, float _major) {
    LOG(DEBUG) << __func__;

    device_->extCmd(device_, kCommandFodPressStatus, kParamFodPressed);

    // Request local HBM
    struct disp_local_hbm_req display_local_hbm_request = {
        .base = display_base_primary,
        .local_hbm_value = LHBM_TARGET_BRIGHTNESS_WHITE_1000NIT,
    };
    if (ioctl(disp_fd_.get(), MI_DISP_IOCTL_SET_LOCAL_HBM,
              &display_local_hbm_request) < 0) {
      LOG(ERROR) << "ioctl SET_LOCAL_HBM(WHITE_1000NIT) failed: "
                 << strerror(errno);
    }

    device_->extCmd(device_, kCommandNit, kParamNitFod);
  }

  void onFingerUp() {
    LOG(DEBUG) << __func__;

    device_->extCmd(device_, kCommandFodPressStatus, kParamFodReleased);

    device_->extCmd(device_, kCommandNit, kParamNitNone);

    struct disp_local_hbm_req display_local_hbm_request = {
        .base = display_base_primary,
        .local_hbm_value = LHBM_TARGET_BRIGHTNESS_OFF_FINGER_UP,
    };
    if (ioctl(disp_fd_.get(), MI_DISP_IOCTL_SET_LOCAL_HBM,
              &display_local_hbm_request) < 0) {
      LOG(ERROR) << "ioctl SET_LOCAL_HBM(off) failed: " << strerror(errno);
    }
  }

  void onAcquired(int32_t result, int32_t /* vendor_code */) {
    switch (static_cast<AcquiredInfo>(result)) {
    case AcquiredInfo::GOOD:
    case AcquiredInfo::PARTIAL:
    case AcquiredInfo::INSUFFICIENT:
    case AcquiredInfo::SENSOR_DIRTY:
    case AcquiredInfo::TOO_SLOW:
    case AcquiredInfo::TOO_FAST:
    case AcquiredInfo::TOO_DARK:
    case AcquiredInfo::TOO_BRIGHT:
    case AcquiredInfo::IMMOBILE:
    case AcquiredInfo::LIFT_TOO_SOON:
      onFingerUp();
      break;
    default:
      break;
    }
  }

  void onAuthenticationSucceeded() { onFingerUp(); }

  void onAuthenticationFailed() { onFingerUp(); }

  void cancel() { onFingerUp(); }

private:
  fingerprint_device_t *device_;
  android::base::unique_fd disp_fd_;
};

static UdfpsHandler *create() { return new ZirconUdfpsHandler(); }

static void destroy(UdfpsHandler *handler) { delete handler; }

extern "C" UdfpsHandlerFactory UDFPS_HANDLER_FACTORY = {
    .create = create,
    .destroy = destroy,
};
