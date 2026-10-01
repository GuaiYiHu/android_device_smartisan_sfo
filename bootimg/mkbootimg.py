#!/usr/bin/env python3
# Copyright (C) 2026 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
"""Pack SFO's legacy Qualcomm DT section using the standard Android payloads."""

import argparse
import hashlib
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path


def without_recovery_editor(raw, extra_files=(), remove_editor=True):
    """Remove optional editor files and explicitly unsupported FS tools."""
    parts = []
    offset = 0
    while offset < len(raw):
        start = offset
        header = raw[start:start + 110]
        if header[:6] not in (b"070701", b"070702"):
            raise ValueError("recovery ramdisk is not a newc archive")
        fields = [int(header[i:i + 8], 16) for i in range(6, 110, 8)]
        file_size, name_size = fields[6], fields[11]
        name = raw[start + 110:start + 110 + name_size - 1].decode()
        data_start = (start + 110 + name_size + 3) & ~3
        offset = (data_start + file_size + 3) & ~3
        if name == "TRAILER!!!":
            parts.append(raw[start:])
            break
        editor_file = (name in ("system/bin/nano", "system/lib/libncurses_recovery.so",
                               "system/etc/nano", "system/etc/terminfo")
                       or name.startswith(("system/etc/nano/", "system/etc/terminfo/")))
        if name not in extra_files and not (remove_editor and editor_file):
            parts.append(raw[start:offset])
    return b"".join(parts)


def unsupported_recovery_fs_tools(kernel, root):
    # Use the config embedded in this exact kernel, including for OTA repacking.
    helper = root / "kernel/smartisan/msm8974/scripts/extract-ikconfig"
    config = set(subprocess.check_output(["/bin/sh", str(helper), kernel], text=True,
                                        env={"PATH": "/usr/bin:/bin"}).splitlines())
    files = set()
    families = {
        "CONFIG_F2FS_FS": ("make_f2fs", "mkfs.f2fs", "sload_f2fs", "fsck.f2fs"),
        "CONFIG_EROFS_FS": ("mkfs.erofs", "fsck.erofs", "dump.erofs"),
    }
    for symbol, tools in families.items():
        if not any(symbol + "=" + value in config for value in ("y", "m")):
            files.update("system/bin/" + tool for tool in tools)
    return files


def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--dt", type=Path)
    parser.add_argument("--recovery-xz-armthumb", action="store_true")
    parser.add_argument("--recovery-no-editor", action="store_true")
    parser.add_argument("--recovery-supported-fs-only", action="store_true")
    parser.add_argument("--recovery-xz-tool", type=Path,
                        default=Path("/usr/bin/xz"))
    legacy, remaining = parser.parse_known_args()
    root = Path(__file__).resolve().parents[4]
    sys.path.insert(0, str(root / "system/tools/mkbootimg"))
    import mkbootimg

    sys.argv[1:] = remaining
    args = mkbootimg.parse_cmdline()
    if args.header_version != 0 or args.vendor_boot is not None:
        parser.error("SFO requires a legacy boot image without vendor_boot")

    if legacy.recovery_xz_armthumb:
        # The kernel enables CONFIG_XZ_DEC_ARMTHUMB. Keep CRC32 and the 32 MiB
        # dictionary, adding BCJ filtering for recovery only.
        # The build's Python omits _lzma and its PATH xz omits the ARM-Thumb
        # encoder. Use the full host xz-utils installation explicitly.
        xz = str(legacy.recovery_xz_tool)
        raw = subprocess.check_output([xz, "--decompress", "--stdout"],
                                      input=args.ramdisk.read())
        extra_files = (unsupported_recovery_fs_tools(args.kernel.name, root)
                       if legacy.recovery_supported_fs_only else ())
        if legacy.recovery_no_editor or extra_files:
            raw = without_recovery_editor(raw, extra_files, legacy.recovery_no_editor)
        args.ramdisk.close()
        args.ramdisk = tempfile.TemporaryFile()
        subprocess.run([xz, "--compress", "--stdout", "--threads=1",
                        "--check=crc32", "--armthumb",
                        "--lzma2=preset=9e,dict=32MiB,lc=1,lp=1,pb=2,nice=128"],
                       input=raw, stdout=args.ramdisk, check=True)
        args.ramdisk.seek(0)

    dt_path = legacy.dt or Path(args.kernel.name).parent / "dt.img"
    if not dt_path.is_file() and legacy.dt is None:
        # Releasetools can rebuild the recovery two-step image from target-files.
        dt_path = Path(os.environ.get("OUT_DIR", "out")) / "target/product/sfo/dt.img"
        if not dt_path.is_absolute():
            dt_path = root / dt_path
    dt = dt_path.read_bytes()
    if len(dt) < 16 or dt[:4] != b"QCDT":
        parser.error("missing Qualcomm device tree table")
    version, entries = struct.unpack_from("<2I", dt, 4)
    if version != 2 or not entries or 12 + entries * 24 + 4 > len(dt):
        parser.error("SFO requires a valid QCDT v2 table")
    for index in range(entries):
        _, _, _, _, offset, size = struct.unpack_from("<6I", dt, 12 + index * 24)
        if offset % args.pagesize or not size or offset + size > len(dt):
            parser.error("invalid QCDT entry bounds/alignment")
        if dt[offset:offset + 4] != b"\xd0\x0d\xfe\xed":
            parser.error("QCDT entry does not contain an FDT")

    # Header v0's later 'header_version' word occupies the original dt_size slot.
    # Keep all standard addresses, command line, OS version and payload encoding,
    # then populate the legacy dt_size and the four-payload image ID.
    mkbootimg.write_header(args)
    mkbootimg.write_data(args, args.pagesize)
    sha = hashlib.sha1()
    for payload in (args.kernel, args.ramdisk, args.second):
        if payload is not None:
            payload.seek(0)
        mkbootimg.update_sha(sha, payload)
    sha.update(dt)
    sha.update(struct.pack("<I", len(dt)))
    args.output.seek(40)
    args.output.write(struct.pack("<I", len(dt)))
    args.output.seek(576)
    args.output.write(sha.digest().ljust(32, b"\0"))
    args.output.seek(0, os.SEEK_END)
    args.output.write(dt)
    mkbootimg.pad_file(args.output, args.pagesize)
    args.output.close()
    if args.ramdisk is not None:
        args.ramdisk.close()


if __name__ == "__main__":
    main()
