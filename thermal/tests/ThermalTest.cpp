// SPDX-License-Identifier: Apache-2.0
#include "Thermal.h"
#include <aidl/android/hardware/thermal/BnThermalChangedCallback.h>
#include <gtest/gtest.h>

#include <chrono>
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <map>

namespace thermal = aidl::android::hardware::thermal;
using thermal::sfo::Thermal;
namespace fs = std::filesystem;
using namespace std::chrono_literals;

class Callback : public thermal::BnThermalChangedCallback {
  public:
    ndk::ScopedAStatus notifyThrottling(const thermal::Temperature& value) override {
        std::lock_guard<std::mutex> lock(mMutex);
        mValues[value.name] = value;
        mWake.notify_all();
        return ndk::ScopedAStatus::ok();
    }
    bool waitFor(const std::string& name, thermal::ThrottlingSeverity severity) {
        std::unique_lock<std::mutex> lock(mMutex);
        return mWake.wait_for(lock, 4s, [&] {
            auto value = mValues.find(name);
            return value != mValues.end() && value->second.throttlingStatus == severity;
        });
    }
    std::vector<thermal::Temperature> values() {
        std::lock_guard<std::mutex> lock(mMutex);
        std::vector<thermal::Temperature> result;
        for (const auto& [name, value] : mValues) result.push_back(value);
        return result;
    }
  private:
    std::mutex mMutex;
    std::condition_variable mWake;
    std::map<std::string, thermal::Temperature> mValues;
};

class SfoThermalTest : public testing::Test {
  protected:
    void SetUp() override {
        char directory[] = "/data/local/tmp/sfo-thermal-test-XXXXXX";
        const char* path = mkdtemp(directory);
        ASSERT_NE(nullptr, path);
        root = path;
        for (int cpu = 0; cpu < 4; ++cpu) {
            zone(cpu + 20, "tsens_tz_sensor" + std::to_string(cpu + 5), "40");
        }
        zone(1, "tsens_tz_sensor3", "39");
        zone(2, "pm8941_tz", "34557");
        write("class/thermal/thermal_zone2/trip_point_0_temp", "145000");
        write("class/thermal/thermal_zone2/trip_point_1_temp", "125000");
        write("class/thermal/thermal_zone2/trip_point_2_temp", "105000");
        zone(3, "pa_therm0", "30");
        zone(4, "pa_therm1", "-40");
        zone(5, "back_battery", "-30000");
        write("class/power_supply/main_battery/present", "1");
        write("class/power_supply/main_battery/temp", "316");
        write("class/power_supply/main_battery/health", "Good");
        write("class/power_supply/main_battery/temp_warm", "450");
        write("class/power_supply/main_battery/temp_cool", "100");
        write("module/msm_thermal/core_control/cpus_offlined", "0");
        write("class/kgsl/kgsl-3d0/gpu_available_frequencies", "578000000 462400000 200000000");
        write("class/kgsl/kgsl-3d0/max_gpuclk", "578000000");
        write("devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies", "300000 960000 2457600");
        write("devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "2457600");
        hal = ndk::SharedRefBase::make<Thermal>(root.string());
    }
    void TearDown() override {
        hal.reset();  // Stop monitor before removing its input directory.
        fs::remove_all(root);
    }
    void write(const std::string& path, const std::string& value) {
        const fs::path file = root / path;
        fs::create_directories(file.parent_path());
        // Atomic writes also keep the monitor from seeing truncated fake sysfs.
        std::ofstream(file.string() + ".new") << value << '\n';
        fs::rename(file.string() + ".new", file);
    }
    void zone(int index, const std::string& type, const std::string& value) {
        const std::string path = "class/thermal/thermal_zone" + std::to_string(index);
        write(path + "/type", type);
        write(path + "/temp", value);
    }
    thermal::Temperature temperature(const std::string& name) {
        std::vector<thermal::Temperature> values;
        EXPECT_TRUE(hal->getTemperatures(&values).isOk());
        for (const auto& value : values) if (value.name == name) return value;
        ADD_FAILURE() << "Missing sensor " << name;
        return {};
    }
    fs::path root;
    std::shared_ptr<Thermal> hal;
};

TEST_F(SfoThermalTest, UnitsMappingAndFiltering) {
    EXPECT_FLOAT_EQ(40, temperature("cpu0").value);
    EXPECT_FLOAT_EQ(34.557f, temperature("pm8941").value);
    EXPECT_FLOAT_EQ(31.6f, temperature("main-battery").value);
    std::vector<thermal::Temperature> values;
    ASSERT_TRUE(hal->getTemperatures(&values).isOk());
    EXPECT_EQ(8u, values.size());
    ASSERT_TRUE(hal->getTemperaturesWithType(thermal::TemperatureType::CPU, &values).isOk());
    EXPECT_EQ(4u, values.size());
    for (const auto& value : values) EXPECT_EQ(thermal::TemperatureType::CPU, value.type);
    ASSERT_TRUE(hal->getTemperaturesWithType(thermal::TemperatureType::SKIN, &values).isOk());
    EXPECT_TRUE(values.empty());
}

TEST_F(SfoThermalTest, OriginalPolicyHysteresisAndReadFailure) {
    const std::string path = "class/thermal/thermal_zone20/temp";
    write(path, "70");
    EXPECT_EQ(thermal::ThrottlingSeverity::LIGHT, temperature("cpu0").throttlingStatus);
    write(path, "60");
    EXPECT_EQ(thermal::ThrottlingSeverity::LIGHT, temperature("cpu0").throttlingStatus);
    write(path, "55");
    EXPECT_EQ(thermal::ThrottlingSeverity::NONE, temperature("cpu0").throttlingStatus);
    write(path, "110");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("cpu0").throttlingStatus);
    write(path, "95");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("cpu0").throttlingStatus);
    write(path, "115");
    EXPECT_EQ(thermal::ThrottlingSeverity::SHUTDOWN, temperature("cpu0").throttlingStatus);
    write(path, "invalid");
    EXPECT_TRUE(std::isnan(temperature("cpu0").value));
    EXPECT_EQ(thermal::ThrottlingSeverity::SHUTDOWN, temperature("cpu0").throttlingStatus);
    write(path, "110");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("cpu0").throttlingStatus);
    write(path, "90");
    EXPECT_EQ(thermal::ThrottlingSeverity::LIGHT, temperature("cpu0").throttlingStatus);
    write(path, "55");
    EXPECT_EQ(thermal::ThrottlingSeverity::NONE, temperature("cpu0").throttlingStatus);
}

TEST_F(SfoThermalTest, ListsAndThresholdsStayStaticWhenOffline) {
    std::vector<thermal::TemperatureThreshold> before, after;
    ASSERT_TRUE(hal->getTemperatureThresholds(&before).isOk());
    fs::remove(root / "class/thermal/thermal_zone20/temp");
    write("class/power_supply/main_battery/present", "0");
    EXPECT_TRUE(std::isnan(temperature("cpu0").value));
    EXPECT_TRUE(std::isnan(temperature("main-battery").value));
    ASSERT_TRUE(hal->getTemperatureThresholds(&after).isOk());
    ASSERT_EQ(before.size(), after.size());
    for (size_t i = 0; i < before.size(); ++i) {
        EXPECT_EQ(before[i].name, after[i].name);
        ASSERT_EQ(7u, after[i].hotThrottlingThresholds.size());
        ASSERT_EQ(7u, after[i].coldThrottlingThresholds.size());
        for (size_t level = 0; level < 7; ++level) {
            const float old = before[i].hotThrottlingThresholds[level];
            const float value = after[i].hotThrottlingThresholds[level];
            if (std::isnan(old)) EXPECT_TRUE(std::isnan(value));
            else EXPECT_FLOAT_EQ(old, value);
        }
    }
}

TEST_F(SfoThermalTest, ChargerHealthAndRealCoolingState) {
    write("class/power_supply/main_battery/health", "Warm");
    EXPECT_EQ(thermal::ThrottlingSeverity::LIGHT, temperature("main-battery").throttlingStatus);
    write("class/power_supply/main_battery/health", "Cold");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("main-battery").throttlingStatus);
    write("class/power_supply/main_battery/health", "Good");
    EXPECT_EQ(thermal::ThrottlingSeverity::NONE, temperature("main-battery").throttlingStatus);
    write("module/msm_thermal/core_control/cpus_offlined", "10");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("cpu1").throttlingStatus);
    EXPECT_EQ(thermal::ThrottlingSeverity::NONE, temperature("cpu0").throttlingStatus);
    write("module/msm_thermal/core_control/cpus_offlined", "invalid");
    EXPECT_EQ(thermal::ThrottlingSeverity::SEVERE, temperature("cpu1").throttlingStatus);
    write("module/msm_thermal/core_control/cpus_offlined", "10");
    write("class/kgsl/kgsl-3d0/max_gpuclk", "462400000");
    write("devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "960000");
    std::vector<thermal::CoolingDevice> cooling;
    ASSERT_TRUE(hal->getCoolingDevices(&cooling).isOk());
    ASSERT_EQ(3u, cooling.size());
    EXPECT_EQ(2, cooling[0].value);
    EXPECT_EQ(1, cooling[1].value);
    EXPECT_EQ(1, cooling[2].value);
    fs::remove(root / "class/kgsl/kgsl-3d0/max_gpuclk");
    EXPECT_EQ(EX_ILLEGAL_STATE, hal->getCoolingDevices(&cooling).getExceptionCode());
    EXPECT_TRUE(cooling.empty());
}

TEST_F(SfoThermalTest, CallbackValidationInitialSnapshotAndTransitions) {
    // Let the monitor fill its cache before registration; the initial snapshot
    // still has to arrive even when every temperature remains constant.
    std::this_thread::sleep_for(100ms);
    auto callback = ndk::SharedRefBase::make<Callback>();
    EXPECT_EQ(EX_ILLEGAL_ARGUMENT,
              hal->registerThermalChangedCallback(nullptr).getExceptionCode());
    ASSERT_TRUE(hal->registerThermalChangedCallbackWithType(callback, thermal::TemperatureType::CPU).isOk());
    EXPECT_EQ(EX_ILLEGAL_ARGUMENT, hal->registerThermalChangedCallback(callback).getExceptionCode());
    ASSERT_TRUE(callback->waitFor("cpu3", thermal::ThrottlingSeverity::NONE));
    for (const auto& value : callback->values()) EXPECT_EQ(thermal::TemperatureType::CPU, value.type);
    write("class/thermal/thermal_zone20/temp", "70");
    ASSERT_TRUE(callback->waitFor("cpu0", thermal::ThrottlingSeverity::LIGHT));
    write("class/thermal/thermal_zone20/temp", "55");
    ASSERT_TRUE(callback->waitFor("cpu0", thermal::ThrottlingSeverity::NONE));
    EXPECT_TRUE(hal->unregisterThermalChangedCallback(callback).isOk());
    EXPECT_EQ(EX_ILLEGAL_ARGUMENT, hal->unregisterThermalChangedCallback(callback).getExceptionCode());
    EXPECT_EQ(EX_ILLEGAL_ARGUMENT, hal->unregisterThermalChangedCallback(nullptr).getExceptionCode());
}

TEST_F(SfoThermalTest, NoSensorsReturnsReadableError) {
    auto empty = ndk::SharedRefBase::make<Thermal>((root / "missing").string());
    std::vector<thermal::Temperature> values;
    auto status = empty->getTemperatures(&values);
    EXPECT_EQ(EX_ILLEGAL_STATE, status.getExceptionCode());
    EXPECT_STRNE("", status.getMessage());
}

class ReentrantCallback : public thermal::BnThermalChangedCallback {
  public:
    explicit ReentrantCallback(std::weak_ptr<Thermal> hal) : mHal(std::move(hal)) {}
    ndk::ScopedAStatus notifyThrottling(const thermal::Temperature&) override {
        if (mCalled.exchange(true)) return ndk::ScopedAStatus::ok();
        auto hal = mHal.lock();
        std::vector<thermal::Temperature> values;
        const bool ok = hal && hal->getTemperatures(&values).isOk() &&
                        hal->unregisterThermalChangedCallback(ref<ReentrantCallback>()).isOk();
        done.set_value(ok);
        return ndk::ScopedAStatus::ok();
    }
    std::promise<bool> done;
  private:
    std::weak_ptr<Thermal> mHal;
    std::atomic<bool> mCalled = false;
};

TEST_F(SfoThermalTest, CallbackCanQueryAndUnregisterWithoutDeadlock) {
    auto callback = ndk::SharedRefBase::make<ReentrantCallback>(hal);
    auto done = callback->done.get_future();
    ASSERT_TRUE(hal->registerThermalChangedCallback(callback).isOk());
    ASSERT_EQ(std::future_status::ready, done.wait_for(4s));
    EXPECT_TRUE(done.get());
}
