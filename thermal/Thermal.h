// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "Sensors.h"

#include <aidl/android/hardware/thermal/BnThermal.h>
#include <condition_variable>
#include <optional>
#include <thread>

namespace aidl::android::hardware::thermal::sfo {

class Thermal : public BnThermal {
  public:
    explicit Thermal(std::string root = "/sys");
    ~Thermal() override;
    ndk::ScopedAStatus getCoolingDevices(std::vector<CoolingDevice>* out) override;
    ndk::ScopedAStatus getCoolingDevicesWithType(CoolingType type,
                                                std::vector<CoolingDevice>* out) override;
    ndk::ScopedAStatus getTemperatures(std::vector<Temperature>* out) override;
    ndk::ScopedAStatus getTemperaturesWithType(TemperatureType type,
                                              std::vector<Temperature>* out) override;
    ndk::ScopedAStatus getTemperatureThresholds(std::vector<TemperatureThreshold>* out) override;
    ndk::ScopedAStatus getTemperatureThresholdsWithType(
            TemperatureType type, std::vector<TemperatureThreshold>* out) override;
    ndk::ScopedAStatus registerThermalChangedCallback(
            const std::shared_ptr<IThermalChangedCallback>& callback) override;
    ndk::ScopedAStatus registerThermalChangedCallbackWithType(
            const std::shared_ptr<IThermalChangedCallback>& callback, TemperatureType type) override;
    ndk::ScopedAStatus unregisterThermalChangedCallback(
            const std::shared_ptr<IThermalChangedCallback>& callback) override;
    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

  private:
    struct Callback {
        uint64_t id;
        std::shared_ptr<IThermalChangedCallback> target;
        std::optional<TemperatureType> type;
        bool initial = true;
    };
    ndk::ScopedAStatus ready() const;
    ndk::ScopedAStatus registerCallback(const std::shared_ptr<IThermalChangedCallback>& callback,
                                       std::optional<TemperatureType> type);
    void monitor();
    ::sfo::Sensors mSensors;
    std::mutex mMutex;
    std::condition_variable mWake;
    bool mStop = false;
    uint64_t mNextId = 0;
    std::vector<Callback> mCallbacks;
    std::thread mMonitor;
};
}  // namespace aidl::android::hardware::thermal::sfo
