/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2019, The MoKee Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstring>
#include <string>

#include <android-base/file.h>
#include <android-base/logging.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"

namespace {

constexpr char kCpuIdPath[] = "/sys/pversion_info/cpu_id";

void OverrideProperty(const char *name, const char *value) {
  auto *property = const_cast<prop_info *>(__system_property_find(name));
  int result;

  if (property != nullptr) {
    result = __system_property_update(property, value, std::strlen(value));
  } else {
    result = __system_property_add(name, std::strlen(name), value,
                                   std::strlen(value));
  }

  if (result != 0) {
    LOG(ERROR) << "Unable to override property " << name;
  }
}

void LoadDevice(const char *name, const char *model, const char *description,
                const char *fingerprint) {
  OverrideProperty("ro.bootimage.build.fingerprint", fingerprint);
  OverrideProperty("ro.build.description", description);
  OverrideProperty("ro.build.fingerprint", fingerprint);
  OverrideProperty("ro.product.name", name);
  OverrideProperty("ro.product.model", model);
  OverrideProperty("ro.vendor.product.model", model);
  OverrideProperty("ro.vendor.build.fingerprint", fingerprint);
}

void LoadProductProperties() {
  std::string cpu_id;
  if (!android::base::ReadFileToString(kCpuIdPath, &cpu_id)) {
    LOG(ERROR) << "Unable to read CPU ID from " << kCpuIdPath;
    return;
  }

  if (cpu_id.find("cpu_id=213") != std::string::npos) {
    LoadDevice(
        "msm8974sfo", "SM701", "msm8974sfo-user 4.4.2 SANFRANCISCO dev-keys",
        "smartisan/msm8974sfo/msm8974sfo:4.4.2/SANFRANCISCO:user/dev-keys");
  } else if (cpu_id.find("cpu_id=194") != std::string::npos) {
    LoadDevice("msm8974sfo_lte", "SM705",
               "msm8974sfo_lte-user 4.4.2 SANFRANCISCO dev-keys",
               "smartisan/msm8974sfo_lte/msm8974sfo_lte:4.4.2/"
               "SANFRANCISCO:user/dev-keys");
  } else {
    LOG(ERROR) << "Unknown CPU ID: " << cpu_id;
  }
}

} // namespace

void vendor_load_properties() {
  LOG(INFO) << "Loading Smartisan SFO variant properties";
  LoadProductProperties();
}
