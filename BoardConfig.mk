#
# Copyright (C) 2015 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
BOARD_VENDOR := smartisan

DEVICE_PATH := device/smartisan/sfo

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := MSM8974
TARGET_NO_BOOTLOADER         := true
TARGET_NO_RADIOIMAGE         := true

# Platform
TARGET_BOARD_PLATFORM     := msm8974
TARGET_BOARD_PLATFORM_GPU := qcom-adreno330

# Architecture
TARGET_ARCH         := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_ABI      := armeabi-v7a
TARGET_CPU_ABI2     := armeabi
TARGET_CPU_SMP      := true
TARGET_CPU_VARIANT  := generic
TARGET_CPU_VARIANT_RUNTIME := krait

# Binder API version
TARGET_USES_64_BIT_BINDER := true

# Extended filesystem support
TARGET_EXFAT_DRIVER := sdfat

# Flags
BOARD_GLOBAL_CFLAGS   += -D__ARM_USE_PLD -D__ARM_CACHE_LINE_SIZE=64

# Filesystem
TARGET_FS_CONFIG_GEN := $(DEVICE_PATH)/config.fs

# Fonts
USE_REDUCED_CJK_FONT_WEIGHTS := true

# Kernel
BOARD_KERNEL_CMDLINE               := console=none vmalloc=340M androidboot.hardware=qcom msm_rtb.filter=0x3b7 ehci-hcd.park=3 androidboot.bootdevice=msm_sdcc.1 androidboot.selinux=permissive
LZMA_RAMDISK_TARGETS               := boot,recovery
BOARD_KERNEL_SEPARATED_DT          := true
BOARD_KERNEL_BASE                  := 0x00000000
BOARD_KERNEL_PAGESIZE              := 2048
BOARD_KERNEL_IMAGE_NAME            := zImage
BOARD_MKBOOTIMG_ARGS               := --ramdisk_offset 0x01000000 --tags_offset 0x00000100
TARGET_KERNEL_SOURCE               := kernel/smartisan/msm8974
TARGET_KERNEL_ARCH                 := arm
TARGET_KERNEL_CONFIG               := mokee_sfo-lte_defconfig
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-linux-androideabi-
BOARD_DTBTOOL_ARGS                 := -2

# QCOM hardware
BOARD_USES_QCOM_HARDWARE            := true

# Power
TARGET_USES_INTERACTION_BOOST := true

# Audio
BOARD_USES_ALSA_AUDIO                      := true
USE_CUSTOM_AUDIO_POLICY                    := 1
USE_XML_AUDIO_POLICY_CONF := 1
TARGET_USES_QCOM_MM_AUDIO                  := true
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP        := true
AUDIO_FEATURE_ENABLED_EXTENDED_COMPRESS_FORMAT := true
AUDIO_FEATURE_ENABLED_EXTN_FORMATS         := true
AUDIO_FEATURE_ENABLED_MULTIPLE_TUNNEL      := true
AUDIO_FEATURE_ENABLED_EXTN_POST_PROC       := true
AUDIO_FEATURE_ENABLED_ANC_HEADSET          := false
AUDIO_FEATURE_ENABLED_FLUENCE              := true
AUDIO_FEATURE_ENABLED_HFP                  := true
AUDIO_FEATURE_ENABLED_PROXY_DEVICE         := true
AUDIO_FEATURE_ENABLED_USBAUDIO             := true
AUDIO_FEATURE_ENABLED_SPKR_PROTECTION      := true
AUDIO_FEATURE_ENABLED_MULTI_VOICE_SESSIONS := true

# Bluetooth
BOARD_HAVE_BLUETOOTH                        := true
BOARD_HAVE_BLUETOOTH_QCOM                   := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth
QCOM_BT_USE_SMD_TTY                         := true
BLUETOOTH_HCI_USE_MCT                       := true

# Radio
FEATURE_QCRIL_UIM_SAP_SERVER_MODE := true
TARGET_USES_OLD_MNC_FORMAT        := true

# Graphics
USE_OPENGL_RENDERER               := true
TARGET_CONTINUOUS_SPLASH_ENABLED  := true
TARGET_USES_C2D_COMPOSITION       := true
TARGET_USE_COMPAT_GRALLOC_PERFORM := true
TARGET_USES_ION                   := true
OVERRIDE_RS_DRIVER                := libRSDriver_adreno.so
NUM_FRAMEBUFFER_SURFACE_BUFFERS   := 3
TARGET_ADDITIONAL_GRALLOC_10_USAGE_BITS := 0x02000000U

# Shader cache config options
# Maximum size of the  GLES Shaders that can be cached for reuse.
# Increase the size if shaders of size greater than 12KB are used.
MAX_EGL_CACHE_KEY_SIZE := 12*1024

# Maximum GLES shader cache size for each app to store the compiled shader
# binaries. Decrease the size if RAM or Flash Storage size is a limitation
# of the device.
MAX_EGL_CACHE_SIZE := 2048*1024

# Camera
TARGET_NEEDS_LEGACY_CAMERA_HAL1_DYN_NATIVE_HANDLE := true
TARGET_USES_MEDIA_EXTENSIONS           := true
TARGET_NEEDS_PLATFORM_TEXT_RELOCATIONS := true
USE_DEVICE_SPECIFIC_CAMERA             := true
TARGET_PROCESS_SDK_VERSION_OVERRIDE := \
    /system/bin/cameraserver=19 \
    /system/bin/mediaserver=19 \
    /system/vendor/bin/mm-qcamera-daemon=19

# Wifi
BOARD_HAS_QCOM_WLAN              := true
BOARD_WLAN_DEVICE                := qcwcn
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
PRODUCT_VENDOR_MOVE_ENABLED      := true
WIFI_DRIVER_FW_PATH_STA          := "sta"
WIFI_DRIVER_FW_PATH_AP           := "ap"
WIFI_HIDL_FEATURE_DISABLE_AP_MAC_RANDOMIZATION := true
TARGET_USES_QCOM_WCNSS_QMI       := true
TARGET_USES_WCNSS_CTRL           := true

# Filesystem
TARGET_USERIMAGES_USE_EXT4          := true
TARGET_USERIMAGES_USE_F2FS          := true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE   := ext4
BOARD_PERSISTIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_BOOTIMAGE_PARTITION_SIZE      := 16384000
BOARD_RECOVERYIMAGE_PARTITION_SIZE  := 16384000
BOARD_SYSTEMIMAGE_PARTITION_SIZE    := 1342177280
BOARD_USERDATAIMAGE_PARTITION_SIZE  := 13291503000
BOARD_CACHEIMAGE_PARTITION_SIZE     := 393216000
BOARD_PERSISTIMAGE_PARTITION_SIZE   := 16384000
BOARD_FLASH_BLOCK_SIZE              := 131072
BOARD_ROOT_EXTRA_FOLDERS           := firmware persist


# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/manifest.xml
DEVICE_MATRIX_FILE := $(DEVICE_PATH)/compatibility_matrix.xml

# Recovery
RECOVERY_FSTAB_VERSION             := 2
TARGET_RECOVERY_DENSITY            := xhdpi
TARGET_RECOVERY_FSTAB              := $(DEVICE_PATH)/rootdir/etc/fstab.qcom
TARGET_RECOVERY_PIXEL_FORMAT       := "RGBX_8888"
TARGET_RECOVERY_LCD_BACKLIGHT_PATH := \"/sys/class/leds/lcd-backlight/brightness\"

# GPS HAL lives here
TARGET_GPS_HAL_PATH         := $(DEVICE_PATH)/gps
TARGET_PROVIDES_GPS_LOC_API := true

# Lights
TARGET_PROVIDES_LIBLIGHT := true

# ANT+
BOARD_ANT_WIRELESS_DEVICE := "vfs-prerelease"

# Use HW crypto for ODE
TARGET_HW_DISK_ENCRYPTION := true
TARGET_LEGACY_HW_DISK_ENCRYPTION := true

# Simple time service client
BOARD_USES_QC_TIME_SERVICES := true

# Charger
BOARD_CHARGER_ENABLE_SUSPEND := true
BOARD_CHARGER_DISABLE_INIT_BLANK := true

# Remove secdiscard command
TARGET_REMOVE_SECDISCARD_COMMAND := true

# Enable dex-preoptimization to speed up first boot sequence
ifeq ($(HOST_OS),linux)
  ifeq ($(TARGET_BUILD_VARIANT),user)
    ifeq ($(WITH_DEXPREOPT),)
      WITH_DEXPREOPT := true
      WITH_DEXPREOPT_BOOT_IMG_AND_SYSTEM_SERVER_ONLY := true
    endif
  endif
endif
DONT_DEXPREOPT_PREBUILTS := true

# Security patch level
VENDOR_SECURITY_PATCH := 2016-07-01

# qcom sepolicy
include device/qcom/sepolicy-legacy/sepolicy.mk

BOARD_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy

-include vendor/smartisan/sfo/BoardConfigVendor.mk
