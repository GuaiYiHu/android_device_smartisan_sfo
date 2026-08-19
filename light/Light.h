/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2019 The MoKee Open Source Project
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/light/2.0/ILight.h>

#include <mutex>

namespace android::hardware::light::V2_0::implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;

class Light : public ILight {
  public:
    Return<Status> setLight(Type type, const LightState& state) override;
    Return<void> getSupportedTypes(getSupportedTypes_cb hidlCb) override;

  private:
    void setBacklight(const LightState& state);
    void setButtons(const LightState& state);
    void setAttention(const LightState& state);
    void setNotifications(const LightState& state);
    void setBattery(const LightState& state);
    void updateIndicatorLocked();
    void setIndicatorLocked(const LightState& state);

    std::mutex mLock;
    LightState mAttention = {};
    LightState mButtons = {};
    LightState mNotification = {};
    LightState mBattery = {};
};

}  // namespace android::hardware::light::V2_0::implementation
