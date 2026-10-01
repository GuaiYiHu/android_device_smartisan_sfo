// SPDX-License-Identifier: Apache-2.0
#include "Thermal.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <utility>

namespace aidl::android::hardware::thermal::sfo {
namespace {
TemperatureType typeOf(::sfo::SensorKind kind) {
    switch (kind) {
        case ::sfo::SensorKind::CPU: return TemperatureType::CPU;
        case ::sfo::SensorKind::BATTERY: return TemperatureType::BATTERY;
        case ::sfo::SensorKind::POWER_AMPLIFIER: return TemperatureType::POWER_AMPLIFIER;
        case ::sfo::SensorKind::OTHER: return TemperatureType::UNKNOWN;
    }
    return TemperatureType::UNKNOWN;
}
bool equal(const std::shared_ptr<IThermalChangedCallback>& a,
           const std::shared_ptr<IThermalChangedCallback>& b) {
    return a && b && a->asBinder() == b->asBinder();
}
ndk::ScopedAStatus invalid(const char* message) {
    return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT, message);
}
bool changed(const Temperature& a, const Temperature& b) {
    return a.throttlingStatus != b.throttlingStatus ||
           std::isfinite(a.value) != std::isfinite(b.value) ||
           (std::isfinite(a.value) && std::fabs(a.value - b.value) >= 0.5f);
}
}  // namespace

Thermal::Thermal(std::string root)
    : mSensors(std::move(root)), mMonitor(&Thermal::monitor, this) {}
Thermal::~Thermal() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStop = true;
    }
    mWake.notify_all();
    mMonitor.join();
}

ndk::ScopedAStatus Thermal::ready() const {
    if (mSensors.initialized()) return ndk::ScopedAStatus::ok();
    return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_STATE,
                                                           "No SFO thermal sensors discovered");
}
ndk::ScopedAStatus Thermal::getTemperatures(std::vector<Temperature>* out) {
    out->clear();
    auto status = ready();
    if (!status.isOk()) return status;
    for (const auto& reading : mSensors.temperatures()) {
        Temperature value;
        value.name = reading.name;
        value.type = typeOf(reading.kind);
        value.value = reading.value;
        value.throttlingStatus = static_cast<ThrottlingSeverity>(reading.severity);
        out->push_back(value);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperaturesWithType(TemperatureType type,
                                                  std::vector<Temperature>* out) {
    auto status = getTemperatures(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperatureThresholds(std::vector<TemperatureThreshold>* out) {
    out->clear();
    auto status = ready();
    if (!status.isOk()) return status;
    for (const auto& reading : mSensors.thresholds()) {
        TemperatureThreshold value;
        value.name = reading.name;
        value.type = typeOf(reading.kind);
        value.hotThrottlingThresholds.assign(reading.hot.begin(), reading.hot.end());
        value.coldThrottlingThresholds.assign(reading.cold.begin(), reading.cold.end());
        out->push_back(value);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperatureThresholdsWithType(
        TemperatureType type, std::vector<TemperatureThreshold>* out) {
    auto status = getTemperatureThresholds(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getCoolingDevices(std::vector<CoolingDevice>* out) {
    out->clear();
    auto status = ready();
    if (!status.isOk()) return status;
    std::vector<::sfo::CoolingReading> readings;
    if (!mSensors.coolingDevices(&readings)) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_STATE,
                                                               "SFO cooling state unavailable");
    }
    for (const auto& reading : readings) {
        CoolingDevice value;
        value.name = reading.name;
        value.type = reading.gpu ? CoolingType::GPU : CoolingType::CPU;
        value.value = reading.value;
        out->push_back(value);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getCoolingDevicesWithType(CoolingType type,
                                                    std::vector<CoolingDevice>* out) {
    auto status = getCoolingDevices(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::registerCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback,
        std::optional<TemperatureType> type) {
    if (!callback) return invalid("Null thermal callback");
    auto status = ready();
    if (!status.isOk()) return status;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& registered : mCallbacks) {
            if (equal(registered.target, callback)) return invalid("Thermal callback already registered");
        }
        mCallbacks.push_back({++mNextId, callback, type});
    }
    mWake.notify_all();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::registerThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback) {
    return registerCallback(callback, std::nullopt);
}
ndk::ScopedAStatus Thermal::registerThermalChangedCallbackWithType(
        const std::shared_ptr<IThermalChangedCallback>& callback, TemperatureType type) {
    return registerCallback(callback, type);
}
ndk::ScopedAStatus Thermal::unregisterThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback) {
    if (!callback) return invalid("Null thermal callback");
    std::lock_guard<std::mutex> lock(mMutex);
    auto match = std::find_if(mCallbacks.begin(), mCallbacks.end(),
                             [&](const auto& c) { return equal(c.target, callback); });
    if (match == mCallbacks.end()) return invalid("Thermal callback was not registered");
    mCallbacks.erase(match);
    return ndk::ScopedAStatus::ok();
}

void Thermal::monitor() {
    std::map<std::string, Temperature> previous;
    for (;;) {
        std::vector<Temperature> current;
        if (getTemperatures(&current).isOk()) {
            std::vector<Callback> callbacks;
            {
                std::lock_guard<std::mutex> lock(mMutex);
                if (mStop) return;
                callbacks = mCallbacks;
                for (auto& callback : mCallbacks) callback.initial = false;
            }
            for (const auto& value : current) {
                auto old = previous.find(value.name);
                const bool notify = old == previous.end() || changed(old->second, value);
                if (notify) previous[value.name] = value;
                for (const auto& callback : callbacks) {
                    if ((!notify && !callback.initial) ||
                        (callback.type && *callback.type != value.type)) continue;
                    // Never call a client while holding the registration/sensor mutex.
                    if (callback.target->notifyThrottling(value).getStatus() == STATUS_DEAD_OBJECT) {
                        std::lock_guard<std::mutex> lock(mMutex);
                        mCallbacks.erase(std::remove_if(mCallbacks.begin(), mCallbacks.end(),
                                                        [&](const auto& c) { return c.id == callback.id; }),
                                         mCallbacks.end());
                    }
                }
            }
        }
        std::unique_lock<std::mutex> lock(mMutex);
        mWake.wait_for(lock, std::chrono::seconds(2), [this] {
            return mStop || std::any_of(mCallbacks.begin(), mCallbacks.end(),
                                        [](const auto& c) { return c.initial; });
        });
        if (mStop) return;
    }
}

binder_status_t Thermal::dump(int fd, const char**, uint32_t) {
    std::vector<Temperature> temperatures;
    auto status = getTemperatures(&temperatures);
    if (!status.isOk()) dprintf(fd, "%s\n", status.getMessage());
    for (const auto& value : temperatures) {
        dprintf(fd, "%s type=%d %.3f C severity=%d\n", value.name.c_str(),
                static_cast<int>(value.type), value.value, static_cast<int>(value.throttlingStatus));
    }
    std::vector<CoolingDevice> cooling;
    status = getCoolingDevices(&cooling);
    if (!status.isOk()) dprintf(fd, "%s\n", status.getMessage());
    for (const auto& value : cooling) {
        dprintf(fd, "%s type=%d state=%lld\n", value.name.c_str(),
                static_cast<int>(value.type), static_cast<long long>(value.value));
    }
    dprintf(fd, "Read-only reporting; stock thermal-engine/kernel own mitigation.\n"
                "No calibrated SKIN sensor: skin headroom/global app status unavailable.\n");
    return STATUS_OK;
}
}  // namespace aidl::android::hardware::thermal::sfo
