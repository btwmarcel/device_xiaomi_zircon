/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vibrator-alias"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

static constexpr const char* source =
        "android.hardware.vibrator.IVibrator/vibratorfeature";
static constexpr const char* target =
        "android.hardware.vibrator.IVibrator/default";

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    AIBinder* vibrator = AServiceManager_waitForService(source);
    CHECK(vibrator != nullptr) << "Could not find " << source;

    binder_status_t status = AServiceManager_addService(vibrator, target);
    CHECK_EQ(status, STATUS_OK) << "Failed to register " << target << ": " << status;
    LOG(INFO) << "Registered " << target << " as an alias of " << target;

    return EXIT_SUCCESS;
}
