// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace sfo {

enum class SensorKind { CPU, BATTERY, POWER_AMPLIFIER, OTHER };
struct SensorReading {
    std::string name;
    SensorKind kind;
    float value;
    int severity;
    std::array<float, 7> hot;
    std::array<float, 7> cold;
};
struct CoolingReading {
    std::string name;
    bool gpu;
    int64_t value;
};

// Read-only reporting. thermal-engine, QPNP and msm_thermal retain mitigation.
class Sensors {
  public:
    explicit Sensors(std::string root = "/sys");
    bool initialized() const { return !mSensors.empty(); }
    std::vector<SensorReading> temperatures();
    std::vector<SensorReading> thresholds() const;
    bool coolingDevices(std::vector<CoolingReading>* readings) const;

  private:
    struct Sensor {
        std::string name;
        SensorKind kind;
        std::string path;
        float divisor;
        std::array<float, 7> hot;
        std::array<float, 7> clear;
        std::array<float, 7> cold;
        int cpu = -1;
        int severity = 0;
    };
    enum class CoolingKind { HOTPLUG, CPU_FREQUENCY, GPU_FREQUENCY };
    struct Cooling {
        std::string name;
        CoolingKind kind;
        std::string path;
        std::vector<int64_t> frequencies;
    };
    const std::string mRoot;
    std::vector<Sensor> mSensors;
    std::vector<Cooling> mCooling;
    int64_t mOfflined = 0;
    std::mutex mMutex;
};
}  // namespace sfo
