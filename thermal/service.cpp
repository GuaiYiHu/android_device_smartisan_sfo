// SPDX-License-Identifier: Apache-2.0
#define LOG_TAG "thermal-sfo"
#include "Thermal.h"
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

using aidl::android::hardware::thermal::sfo::Thermal;

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "--dump-sensors") == 0) {
        const auto thermal = ndk::SharedRefBase::make<Thermal>();
        thermal->dump(STDOUT_FILENO, nullptr, 0);
        std::vector<aidl::android::hardware::thermal::Temperature> readings;
        return thermal->getTemperatures(&readings).isOk() ? 0 : 1;
    }
    if (argc != 1) {
        std::fprintf(stderr, "Usage: %s [--dump-sensors]\n", argv[0]);
        return 1;
    }
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    const auto thermal = ndk::SharedRefBase::make<Thermal>();
    const std::string name = std::string(Thermal::descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(thermal->asBinder().get(), name.c_str()), STATUS_OK);
    LOG(INFO) << "Registered SFO read-only thermal HAL; stock mitigation retained";
    ABinderProcess_joinThreadPool();
    return 1;
}
