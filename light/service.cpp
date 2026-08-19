/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.light@2.0-service.sfo"

#include "Light.h"

#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

using android::OK;
using android::sp;
using android::status_t;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

int main() {
    sp<ILight> service = new Light();

    configureRpcThreadpool(1, true);

    const status_t status = service->registerAsService();
    if (status != OK) {
        ALOGE("Cannot register Light HAL service");
        return 1;
    }

    ALOGI("Light HAL service ready");
    joinRpcThreadpool();

    ALOGE("Light HAL service left the RPC threadpool");
    return 1;
}
