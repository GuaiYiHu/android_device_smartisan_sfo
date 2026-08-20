/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "sfo_wcnss_qmi"

#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAC_ADDRESS_SIZE 6
#define WLAN_MAC_NV_FILE "/persist/.wifi_mac_nv.bin"

static bool is_valid_mac_address(const uint8_t mac[MAC_ADDRESS_SIZE])
{
    bool all_zero = true;
    bool all_ones = true;
    size_t i;

    for (i = 0; i < MAC_ADDRESS_SIZE; ++i) {
        all_zero &= mac[i] == 0x00;
        all_ones &= mac[i] == 0xff;
    }

    return !all_zero && !all_ones && !(mac[0] & 0x01);
}

int wcnss_init_qmi(void)
{
    return 0;
}

int wcnss_qmi_get_wlan_address(unsigned char *address)
{
    uint8_t mac[MAC_ADDRESS_SIZE];
    size_t offset = 0;
    int fd;

    if (address == NULL) {
        ALOGE("Invalid destination for WLAN MAC address");
        return -1;
    }

    fd = open(WLAN_MAC_NV_FILE, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("Failed to open %s: %s", WLAN_MAC_NV_FILE, strerror(errno));
        return -1;
    }

    while (offset < sizeof(mac)) {
        ssize_t bytes_read = read(fd, mac + offset, sizeof(mac) - offset);

        if (bytes_read < 0) {
            if (errno == EINTR)
                continue;

            ALOGE("Failed to read %s: %s", WLAN_MAC_NV_FILE,
                  strerror(errno));
            close(fd);
            return -1;
        }

        if (bytes_read == 0)
            break;

        offset += bytes_read;
    }

    close(fd);

    if (offset != sizeof(mac)) {
        ALOGE("Invalid WLAN MAC data length in %s: %zu",
              WLAN_MAC_NV_FILE, offset);
        return -1;
    }

    if (!is_valid_mac_address(mac)) {
        ALOGE("Invalid WLAN MAC address in %s", WLAN_MAC_NV_FILE);
        return -1;
    }

    memcpy(address, mac, sizeof(mac));
    ALOGI("Loaded factory WLAN MAC address from persist");
    return 0;
}

void wcnss_qmi_deinit(void)
{
}
