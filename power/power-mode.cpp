/*
 * Copyright (C) 2024 The LineageOS Project
 * Copyright (C) 2026 btwmarcel
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <aidl/android/hardware/power/BnPower.h>
#include <android-base/file.h>
#include <android-base/logging.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

constexpr unsigned int kSetCurValue = 0;
constexpr unsigned int kTouchDoubleTapMode = 14;
constexpr unsigned int kTouchMagic = 't';

constexpr unsigned int kTouchIoctlSetMode =
    _IO(kTouchMagic, kSetCurValue);

constexpr const char* kTouchDevPath = "/dev/xiaomi-touch";

constexpr unsigned int kTouchId = 0;

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

using ::aidl::android::hardware::power::Mode;

bool isDeviceSpecificModeSupported(Mode type, bool* _aidl_return) {
    switch (type) {
        case Mode::DOUBLE_TAP_TO_WAKE:
            *_aidl_return = true;
            return true;
        default:
            return false;
    }
}

bool setDeviceSpecificMode(Mode type, bool enabled) {
    switch (type) {
        case Mode::DOUBLE_TAP_TO_WAKE: {
            int fd = open(kTouchDevPath, O_RDWR);
            int arg[3] = {kTouchId, kTouchDoubleTapMode, enabled ? 1 : 0};
            ioctl(fd, kTouchIoctlSetMode, &arg);
            close(fd);
            return true;
        }
        default:
            return false;
    }
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
