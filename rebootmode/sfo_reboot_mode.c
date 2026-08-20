/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ALTERABLE_PATH "/dev/block/platform/msm_sdcc.1/by-name/alterable"
#define SECTOR_SIZE 512
#define REBOOT_MODE_OFFSET 3
#define REBOOT_MODE_MASK 0x0c
#define REBOOT_MODE_BOOTLOADER 0x04
#define REBOOT_MODE_RECOVERY 0x08

static int pread_full(int fd, void *buffer, size_t size, off_t offset)
{
    uint8_t *cursor = buffer;
    size_t done = 0;

    while (done < size) {
        ssize_t count = pread(fd, cursor + done, size - done, offset + done);
        if (count < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (count == 0) {
            errno = EIO;
            return -1;
        }
        done += (size_t)count;
    }

    return 0;
}

static int pwrite_full(int fd, const void *buffer, size_t size, off_t offset)
{
    const uint8_t *cursor = buffer;
    size_t done = 0;

    while (done < size) {
        ssize_t count = pwrite(fd, cursor + done, size - done, offset + done);
        if (count < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (count == 0) {
            errno = EIO;
            return -1;
        }
        done += (size_t)count;
    }

    return 0;
}

int main(int argc, char **argv)
{
    uint8_t sector[SECTOR_SIZE];
    uint8_t mode;
    int fd;

    if (argc != 2) {
        fprintf(stderr, "usage: %s bootloader|recovery|normal\n", argv[0]);
        return 2;
    }

    if (!strcmp(argv[1], "bootloader")) {
        mode = REBOOT_MODE_BOOTLOADER;
    } else if (!strcmp(argv[1], "recovery")) {
        mode = REBOOT_MODE_RECOVERY;
    } else if (!strcmp(argv[1], "normal")) {
        mode = 0;
    } else {
        fprintf(stderr, "unknown reboot mode: %s\n", argv[1]);
        return 2;
    }

    fd = open(ALTERABLE_PATH, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "cannot open %s: %s\n", ALTERABLE_PATH, strerror(errno));
        return 1;
    }

    if (pread_full(fd, sector, sizeof(sector), 0) < 0) {
        fprintf(stderr, "cannot read %s: %s\n", ALTERABLE_PATH, strerror(errno));
        close(fd);
        return 1;
    }

    sector[REBOOT_MODE_OFFSET] =
            (sector[REBOOT_MODE_OFFSET] & ~REBOOT_MODE_MASK) | mode;

    if (pwrite_full(fd, sector, sizeof(sector), 0) < 0 || fsync(fd) < 0) {
        fprintf(stderr, "cannot update %s: %s\n", ALTERABLE_PATH, strerror(errno));
        close(fd);
        return 1;
    }

    if (close(fd) < 0) {
        fprintf(stderr, "cannot close %s: %s\n", ALTERABLE_PATH, strerror(errno));
        return 1;
    }

    return 0;
}
