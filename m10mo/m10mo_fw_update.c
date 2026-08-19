/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "m10mo_fw_update"

#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define M10MO_FUNCTIONS \
    "/sys/class/i2c-dev/i2c-4/device/4-003e/functions"

struct m10mo_firmware {
    const char *vendor;
    const char *version;
    const char *path;
};

static const struct m10mo_firmware firmware_table[] = {
    { "FOXCONN", "C117F", "/vendor/etc/firmware/RS_M10MO_F.fw" },
    { "LITE_ON", "C117L", "/vendor/etc/firmware/RS_M10MO_L.fw" },
};

int main(void)
{
    char info[128] = { 0 };
    char current[16] = { 0 };
    char vendor[32] = { 0 };
    const struct m10mo_firmware *firmware = NULL;
    ssize_t length;
    int fd;
    size_t i;

    fd = open(M10MO_FUNCTIONS, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("cannot open %s: %s", M10MO_FUNCTIONS, strerror(errno));
        return 1;
    }

    length = read(fd, info, sizeof(info) - 1);
    close(fd);
    if (length <= 0) {
        ALOGE("cannot read M10MO firmware information: %s", strerror(errno));
        return 1;
    }

    if (sscanf(info, "%15s %31s", current, vendor) != 2) {
        ALOGE("invalid M10MO firmware information: %s", info);
        return 1;
    }

    for (i = 0; i < sizeof(firmware_table) / sizeof(firmware_table[0]); ++i) {
        if (!strcmp(vendor, firmware_table[i].vendor)) {
            firmware = &firmware_table[i];
            break;
        }
    }

    if (firmware == NULL) {
        ALOGE("unsupported M10MO module vendor: %s", vendor);
        return 1;
    }

    if (!strcmp(current, firmware->version)) {
        ALOGI("M10MO %s firmware is already %s", vendor, current);
        return 0;
    }

    ALOGW("updating M10MO %s firmware from %s to %s", vendor, current,
          firmware->version);
    execl("/vendor/bin/isp_write", "isp_write", "s", firmware->path,
          (char *)NULL);

    ALOGE("cannot execute isp_write: %s", strerror(errno));
    return 1;
}
