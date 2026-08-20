# TWRP device tree for Smartisan T1 (sfo)

This recovery-only tree targets Android 7.1/TWRP and combines:

- Xiaomi cancro's `twrp-7.1` boot-image layout (commit `de09bf3`)
- Smartisan sfo's `twrp/android-9.0` recovery configuration (commit `9bcfa33`)
- locally built LineageOS 18.1 kernel and device-tree images
- the Smartisan `alterable` reboot-mode workaround

## Prebuilt inputs

The checked-in files were copied from:

```text
/home/guaiyihu/Android/lineage-18.1/out/target/product/sfo/kernel
/home/guaiyihu/Android/lineage-18.1/out/target/product/sfo/dt.img
```

Their SHA-256 digests are:

```text
kernel  6b6ce1561e100320b52d55ca9cafadab382b3f577e5b14141a7223cf0c94566f
dt.img  e1f8a4161893629e93300fac7160fc6bf0368792d9372328d967c365ab7f1685
```

When updating either prebuilt, replace the corresponding file under
`prebuilt/` and update these digests.

## Build

This Android 7.1 tree requires Java 8 and Python 2. The following uses the
Python 2.7 prebuilt already present in the source tree, without changing the
host's default `python` command:

```sh
build_python_dir="$(mktemp -d)"
ln -s "$PWD/prebuilts/python/linux-x86/2.7.5/bin/python2.7" \
    "$build_python_dir/python"
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH="$build_python_dir:$JAVA_HOME/bin:$PATH"

source build/envsetup.sh
lunch omni_sfo-eng
mka recoveryimage
```

The resulting image is `out/target/product/sfo/recovery.img`.

## Reboot mode

Smartisan's bootloader reads bits 2-3 at byte offset 3 in the first sector of
the `alterable` partition. `sfo_reboot_mode` is installed in `/sbin`.

TWRP calls `/sbin/rebootrecovery.sh` or `/sbin/rebootbootloader.sh`
synchronously before requesting a reboot. The device-provided hook scripts run
`sfo_reboot_mode`, so the TWRP reboot menu works without replacing the main
recovery `init.rc` or modifying `bootable/recovery`.

The Android system-side `adb reboot recovery` / `adb reboot bootloader`
behavior remains handled by the corresponding handlers in the LineageOS sfo
device tree. These TWRP hook scripts specifically cover reboot requests made
from TWRP itself.

## Battery

The sfo kernel exposes the active pack as
`/sys/class/power_supply/main_battery`, rather than TWRP's default
`/sys/class/power_supply/battery`. `TW_CUSTOM_BATTERY_PATH` selects the actual
node so that both capacity and charging status are shown.
