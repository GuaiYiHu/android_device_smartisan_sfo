/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2012-2014 The Linux Foundation. All rights reserved.
 * Copyright (C) 2019 The MoKee Open Source Project
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "LightServiceSfo"

#include "Light.h"

#include <android-base/file.h>
#include <log/log.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace android::hardware::light::V2_0::implementation {
namespace {

constexpr const char* kLcdBacklight = "/sys/class/leds/lcd-backlight/brightness";

struct IndicatorChannel {
    const char* brightness;
    const char* blink;
    const char* dutyPcts;
    const char* startIndex;
    const char* pauseLo;
    const char* pauseHi;
    const char* rampStepMs;
    int startIndexValue;
};

constexpr int kRampSize = 8;
constexpr int kDefaultRampStepMs = 50;
constexpr std::array<int, kRampSize> kBrightnessRamp = {0, 12, 25, 37, 50, 72, 85, 100};

// SFO's QPNP RGB channels share one LUT, so each channel gets its own range.
constexpr std::array<IndicatorChannel, 3> kIndicatorChannels = {{
        {
                "/sys/class/leds/red/brightness",
                "/sys/class/leds/red/blink",
                "/sys/class/leds/red/duty_pcts",
                "/sys/class/leds/red/start_idx",
                "/sys/class/leds/red/pause_lo",
                "/sys/class/leds/red/pause_hi",
                "/sys/class/leds/red/ramp_step_ms",
                0,
        },
        {
                "/sys/class/leds/green/brightness",
                "/sys/class/leds/green/blink",
                "/sys/class/leds/green/duty_pcts",
                "/sys/class/leds/green/start_idx",
                "/sys/class/leds/green/pause_lo",
                "/sys/class/leds/green/pause_hi",
                "/sys/class/leds/green/ramp_step_ms",
                kRampSize,
        },
        {
                "/sys/class/leds/blue/brightness",
                "/sys/class/leds/blue/blink",
                "/sys/class/leds/blue/duty_pcts",
                "/sys/class/leds/blue/start_idx",
                "/sys/class/leds/blue/pause_lo",
                "/sys/class/leds/blue/pause_hi",
                "/sys/class/leds/blue/ramp_step_ms",
                kRampSize * 2,
        },
}};

uint32_t rgbToBrightness(const LightState& state) {
    const uint32_t color = state.color & 0x00ffffff;
    return (77 * ((color >> 16) & 0xff) + 150 * ((color >> 8) & 0xff) +
            29 * (color & 0xff)) >>
           8;
}

bool isLit(const LightState& state) {
    return state.color & 0x00ffffff;
}

bool writeValue(const char* path, int value) {
    if (android::base::WriteStringToFile(std::to_string(value) + "\n", path)) {
        return true;
    }

    ALOGE("Failed to write %d to %s", value, path);
    return false;
}

bool writeString(const char* path, const std::string& value) {
    if (android::base::WriteStringToFile(value + "\n", path)) {
        return true;
    }

    ALOGE("Failed to write to %s", path);
    return false;
}

std::string getScaledDutyPcts(uint32_t brightness) {
    std::string values;

    for (const int percent : kBrightnessRamp) {
        if (!values.empty()) {
            values += ',';
        }
        values += std::to_string(percent * brightness / 255);
    }

    return values;
}

LightState applyAlpha(const LightState& state) {
    const uint32_t alpha = state.color >> 24;
    if (alpha == 0 || alpha == 255) {
        return state;
    }

    LightState scaled = state;
    const uint32_t color = state.color & 0x00ffffff;
    const uint32_t red = ((color >> 16) & 0xff) * alpha / 255;
    const uint32_t green = ((color >> 8) & 0xff) * alpha / 255;
    const uint32_t blue = (color & 0xff) * alpha / 255;
    scaled.color = (red << 16) | (green << 8) | blue;
    return scaled;
}

}  // namespace

Return<Status> Light::setLight(Type type, const LightState& state) {
    switch (type) {
        case Type::BACKLIGHT:
            setBacklight(state);
            return Status::SUCCESS;
        case Type::BUTTONS:
            setButtons(state);
            return Status::SUCCESS;
        case Type::ATTENTION:
            setAttention(state);
            return Status::SUCCESS;
        case Type::NOTIFICATIONS:
            setNotifications(state);
            return Status::SUCCESS;
        case Type::BATTERY:
            setBattery(state);
            return Status::SUCCESS;
        default:
            return Status::LIGHT_NOT_SUPPORTED;
    }
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb hidlCb) {
    hidlCb(std::vector<Type>{
            Type::BACKLIGHT,
            Type::BUTTONS,
            Type::BATTERY,
            Type::NOTIFICATIONS,
            Type::ATTENTION,
    });
    return Void();
}

void Light::setBacklight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    writeValue(kLcdBacklight, rgbToBrightness(state));
}

void Light::setButtons(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    // The stock HAL exposes this as the private "fake_button" light and
    // multiplexes it onto the RGB indicator instead of a KPD backlight.
    mButtons = state;
    updateIndicatorLocked();
}

void Light::setAttention(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mAttention = state;
    updateIndicatorLocked();
}

void Light::setNotifications(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mNotification = applyAlpha(state);
    updateIndicatorLocked();
}

void Light::setBattery(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mBattery = state;
    updateIndicatorLocked();
}

void Light::updateIndicatorLocked() {
    if (isLit(mNotification)) {
        setIndicatorLocked(mNotification);
    } else if (isLit(mAttention)) {
        setIndicatorLocked(mAttention);
    } else if (isLit(mButtons)) {
        setIndicatorLocked(mButtons);
    } else {
        setIndicatorLocked(mBattery);
    }
}

void Light::setIndicatorLocked(const LightState& state) {
    const uint32_t brightness = rgbToBrightness(state);
    const bool blink = state.flashMode == Flash::TIMED && state.flashOnMs > 0 &&
                       state.flashOffMs > 0;

    // Stop the previous LUT before changing its parameters or using steady mode.
    for (const auto& channel : kIndicatorChannels) {
        writeValue(channel.blink, 0);
    }

    if (!blink) {
        for (const auto& channel : kIndicatorChannels) {
            writeValue(channel.brightness, brightness);
        }
        return;
    }

    int stepMs = kDefaultRampStepMs;
    int pauseHiMs = state.flashOnMs - stepMs * kRampSize * 2;
    if (pauseHiMs < 0) {
        stepMs = std::max(1, state.flashOnMs / (kRampSize * 2));
        pauseHiMs = 0;
    }

    const std::string dutyPcts = getScaledDutyPcts(brightness);
    for (const auto& channel : kIndicatorChannels) {
        writeValue(channel.startIndex, channel.startIndexValue);
        writeString(channel.dutyPcts, dutyPcts);
        writeValue(channel.pauseLo, state.flashOffMs);
        writeValue(channel.pauseHi, pauseHiMs);
        writeValue(channel.rampStepMs, stepMs);
        writeValue(channel.blink, 1);
    }
}

}  // namespace android::hardware::light::V2_0::implementation
