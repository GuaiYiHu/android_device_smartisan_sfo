// SPDX-License-Identifier: Apache-2.0
#include "Sensors.h"

#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <utility>

namespace sfo {
namespace {
constexpr float kUnknown = std::numeric_limits<float>::quiet_NaN();
const std::array<float, 7> kNoThresholds = {
        kUnknown, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown};

bool readNumber(const std::string& path, int64_t* result) {
    std::ifstream input(path);
    return static_cast<bool>(input >> *result);
}
std::string readText(const std::string& path) {
    std::ifstream input(path);
    std::string result;
    std::getline(input, result);
    return result;
}
std::vector<int64_t> readFrequencies(const std::string& path) {
    std::ifstream input(path);
    std::vector<int64_t> frequencies;
    int64_t frequency;
    while (input >> frequency) {
        if (frequency > 0) frequencies.push_back(frequency);
    }
    std::sort(frequencies.begin(), frequencies.end(), std::greater<int64_t>());
    frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
    return frequencies;
}
int severityFor(float value, int previous, const std::array<float, 7>& hot,
                const std::array<float, 7>& clear) {
    while (previous > 0 &&
           (!std::isfinite(clear[previous]) || value <= clear[previous])) --previous;
    for (int level = previous + 1; level < 7; ++level) {
        if (std::isfinite(hot[level]) && value >= hot[level]) previous = level;
    }
    return previous;
}
}  // namespace

Sensors::Sensors(std::string root) : mRoot(std::move(root)) {
    // Zone indices depend on probe order. Resolve the driver's type instead.
    std::map<std::string, std::string> zones;
    const std::string base = mRoot + "/class/thermal/";
    if (DIR* dir = opendir(base.c_str())) {
        while (dirent* entry = readdir(dir)) {
            const std::string name = entry->d_name;
            if (name.compare(0, 12, "thermal_zone") != 0) continue;
            const std::string path = base + name;
            zones.emplace(readText(path + "/type"), path);
        }
        closedir(dir);
    }
    for (int cpu = 0; cpu < 4; ++cpu) {
        const auto zone = zones.find("tsens_tz_sensor" + std::to_string(cpu + 5));
        if (zone == zones.end()) continue;
        // CPU mapping: msm8974.dtsi qcom,cpu-sensors and stock thermal-engine.
        // SS starts at 70 C, clears at 55 C; kernel hotplug/frequency mitigation
        // starts at 110 C, clears at 90 C; stock shutdown is 115/110 C.
        mSensors.push_back({"cpu" + std::to_string(cpu), SensorKind::CPU,
                            zone->second + "/temp", 1.0f,
                            {kUnknown, 70, kUnknown, 110, kUnknown, kUnknown, 115},
                            {kUnknown, 55, kUnknown, 90, kUnknown, kUnknown, 110},
                            kNoThresholds, cpu});
    }
    if (const auto zone = zones.find("tsens_tz_sensor3"); zone != zones.end()) {
        // Stock alias pop_mem is TSENS 3, not a GPU or surface thermometer.
        mSensors.push_back({"pop-mem", SensorKind::OTHER, zone->second + "/temp", 1.0f,
                            {kUnknown, 70, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown},
                            {kUnknown, 55, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown},
                            kNoThresholds});
    }
    for (const std::string type : {"pm8841_tz", "pm8941_tz"}) {
        const auto zone = zones.find(type);
        if (zone == zones.end()) continue;
        auto hot = kNoThresholds;
        auto clear = kNoThresholds;
        // QPNP exposes stage3/stage2/stage1 as trip0/trip1/trip2 in mC.
        // Read the actual configured alarm points, once, for static thresholds.
        for (const auto& [trip, severity] : {std::pair{2, 1}, {1, 3}, {0, 6}}) {
            int64_t raw;
            if (readNumber(zone->second + "/trip_point_" + std::to_string(trip) + "_temp",
                           &raw) && raw > 0 && raw <= 200000) {
                hot[severity] = raw / 1000.0f;
                clear[severity] = hot[severity] - 2.0f;  // QPNP stage hysteresis.
            }
        }
        mSensors.push_back({type == "pm8841_tz" ? "pm8841-alarm" : "pm8941",
                            SensorKind::OTHER, zone->second + "/temp", 1000.0f,
                            hot, clear, kNoThresholds});
    }
    const std::string battery = mRoot + "/class/power_supply/main_battery/";
    int64_t present;
    if (readNumber(battery + "present", &present) && present == 1) {
        auto hot = kNoThresholds;
        auto cold = kNoThresholds;
        int64_t raw;
        if (readNumber(battery + "temp_warm", &raw)) hot[1] = raw / 10.0f;
        if (readNumber(battery + "temp_cool", &raw)) cold[1] = raw / 10.0f;
        // power_supply temperatures are deci-C, unlike the thermal-zone mC.
        // The charger health flag determines severity, including its hysteresis.
        mSensors.push_back({"main-battery", SensorKind::BATTERY, battery + "temp", 10.0f,
                            hot, kNoThresholds, cold});
    }
    if (const auto zone = zones.find("pa_therm0"); zone != zones.end()) {
        int64_t raw;
        if (readNumber(zone->second + "/temp", &raw) && raw > -40 && raw <= 150) {
            mSensors.push_back({"pa-therm0", SensorKind::POWER_AMPLIFIER,
                                zone->second + "/temp", 1.0f,
                                kNoThresholds, kNoThresholds, kNoThresholds});
        }
    }
    std::sort(mSensors.begin(), mSensors.end(),
              [](const Sensor& a, const Sensor& b) { return a.name < b.name; });

    const std::string hotplug = mRoot + "/module/msm_thermal/core_control/cpus_offlined";
    int64_t value;
    if (readNumber(hotplug, &value)) {
        mCooling.push_back({"cpu-thermal-hotplug", CoolingKind::HOTPLUG, hotplug, {}});
    }
    {
        // Other CPUs' cpufreq directories disappear during ordinary hotplug.
        // CPU0 stays online; report its cap rather than a changing device list.
        const std::string path = mRoot + "/devices/system/cpu/cpu0/cpufreq/";
        auto frequencies = readFrequencies(path + "scaling_available_frequencies");
        if (readNumber(path + "scaling_max_freq", &value) && value > 0 && !frequencies.empty()) {
            mCooling.push_back({"cpu0-frequency-cap",
                                CoolingKind::CPU_FREQUENCY, path + "scaling_max_freq",
                                std::move(frequencies)});
        }
    }
    const std::string gpu = mRoot + "/class/kgsl/kgsl-3d0/";
    auto frequencies = readFrequencies(gpu + "gpu_available_frequencies");
    if (readNumber(gpu + "max_gpuclk", &value) && value > 0 && !frequencies.empty()) {
        mCooling.push_back({"gpu-frequency-cap", CoolingKind::GPU_FREQUENCY,
                            gpu + "max_gpuclk", std::move(frequencies)});
    }
}

std::vector<SensorReading> Sensors::temperatures() {
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<SensorReading> readings;
    int64_t offlined;
    if (readNumber(mRoot + "/module/msm_thermal/core_control/cpus_offlined", &offlined) &&
        offlined >= 0) mOfflined = offlined;
    for (Sensor& sensor : mSensors) {
        float value = kUnknown;
        int64_t raw;
        if (readNumber(sensor.path, &raw)) {
            const float candidate = raw / sensor.divisor;
            if (candidate >= -40 && candidate <= 200 &&
                !(sensor.kind == SensorKind::POWER_AMPLIFIER && raw == -40)) value = candidate;
        }
        if (sensor.kind == SensorKind::BATTERY) {
            const std::string battery = mRoot + "/class/power_supply/main_battery/";
            int64_t present;
            if (!readNumber(battery + "present", &present) || present != 1) value = kUnknown;
            if (std::isfinite(value)) {
                const std::string health = readText(battery + "health");
                if (health == "Overheat" || health == "Cold") sensor.severity = 3;
                else if (health == "Warm" || health == "Cool") sensor.severity = 1;
                else if (health == "Good") sensor.severity = 0;
            }
        } else if (std::isfinite(value)) {
            sensor.severity = severityFor(value, sensor.severity, sensor.hot, sensor.clear);
        }
        // A read error must not erase a sensor or falsely clear a protection state.
        int severity = sensor.severity;
        if (sensor.cpu >= 0 && (mOfflined & (int64_t{1} << sensor.cpu))) {
            severity = std::max(severity, 3);
        }
        readings.push_back({sensor.name, sensor.kind, value, severity, sensor.hot, sensor.cold});
    }
    return readings;
}

std::vector<SensorReading> Sensors::thresholds() const {
    std::vector<SensorReading> readings;
    for (const auto& sensor : mSensors) {
        readings.push_back({sensor.name, sensor.kind, kUnknown, 0, sensor.hot, sensor.cold});
    }
    return readings;
}

bool Sensors::coolingDevices(std::vector<CoolingReading>* readings) const {
    readings->clear();
    for (const auto& device : mCooling) {
        int64_t raw;
        if (!readNumber(device.path, &raw) || raw < 0 ||
            (device.kind != CoolingKind::HOTPLUG && raw == 0)) {
            readings->clear();
            return false;
        }
        int64_t state;
        if (device.kind == CoolingKind::HOTPLUG) {
            state = __builtin_popcountll(static_cast<unsigned long long>(raw) & 0xfULL);
        } else {
            const auto allowed = std::find_if(device.frequencies.begin(), device.frequencies.end(),
                                              [raw](int64_t frequency) { return frequency <= raw; });
            state = allowed == device.frequencies.end() ? device.frequencies.size() - 1 :
                                                         allowed - device.frequencies.begin();
        }
        readings->push_back({device.name, device.kind == CoolingKind::GPU_FREQUENCY, state});
    }
    return true;
}
}  // namespace sfo
