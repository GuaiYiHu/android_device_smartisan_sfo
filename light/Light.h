/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2019 The MoKee Open Source Project
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <mutex>
#include <vector>

namespace aidl::android::hardware::light::implementation {
using LightState = HwLightState;
using Type = LightType;
using Flash = FlashMode;

class Light : public BnLights {
  public:
    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

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

}  // namespace aidl::android::hardware::light::implementation
