LOCAL_PATH := device/smartisan/sfo

# Retain the framework matrix for this device's declared FCM level.
PRODUCT_PACKAGES += framework_compatibility_matrix.5.xml

# Android 14 netd requires the cgroup v2 hierarchy at /sys/fs/cgroup.
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/cgroups.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json

DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Ramdisk
PRODUCT_PACKAGES += \
    fstab.qcom \
    init.qcom.rc \
    init.recovery.qcom.rc \
    init.target.rc \
    init.qcom.usb.rc \
    ueventd.qcom.rc \
    vndservicemanager

# Smartisan aboot reads reboot mode flags from the alterable partition.
PRODUCT_PACKAGES += \
    sfo_reboot_mode

# QCOM Config Script
PRODUCT_PACKAGES += \
    init.qcom.bt.sh \
    init.class_main.sh \
    init.qcom.early_boot.sh \
    init.qcom.sh \
    init.qcom.usb.sh \
    init.qcom.post_boot.sh \
    init.program_bdaddr.sh

# GPS
PRODUCT_PACKAGES += \
    gps.msm8974
 
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/gps/etc/gps.conf:system/etc/gps.conf \
    $(LOCAL_PATH)/gps/etc/flp.conf:system/etc/flp.conf \
    $(LOCAL_PATH)/gps/etc/izat.conf:system/etc/izat.conf \
    $(LOCAL_PATH)/gps/etc/sap.conf:system/etc/sap.conf

PRODUCT_PACKAGES += \
    android.hardware.gnss@1.0-impl

# Keymaster
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl

# Camera
PRODUCT_PACKAGES += \
    camera.msm8974 \
    camera.legacy.msm8974 \
    libkkcomp \
    libxml2 \
    m10mo_fw_update \
    Camera2 \
    camera.device@1.0-impl \
    android.hardware.camera.provider@2.4-impl \
    android.hardware.camera.provider@2.4-service

# Dependencies used by legacy vendor camera/display, netmgrd and RIL blobs.
PRODUCT_SOONG_NAMESPACES += device/smartisan/sfo/libshims

PRODUCT_PACKAGES += \
    libgui.vendor \
    libnetutils.vendor \
    libsqlite.vendor \
    libxml2.vendor \
    libsensorndkbridge \
    libstdc++_vendor \
    sfo-libstdc++-link32

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/external_camera_config.xml:system/vendor/etc/external_camera_config.xml

# Charger
PRODUCT_PACKAGES += \
    charger_res_images

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light-service.sfo

# Power
PRODUCT_PACKAGES += \
    android.hardware.power-service.sfo

# Privapp Whitelist
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/privapp-permissions-qti.xml:system/etc/permissions/privapp-permissions-qti.xml

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator-service.sfo \
    vibrator.default

#USB HAL
PRODUCT_PACKAGES += \
    android.hardware.usb@1.3-service.basic

# WiFi
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wifi/WCNSS_cfg.dat:system/vendor/firmware/wlan/prima/WCNSS_cfg.dat \
    $(LOCAL_PATH)/wifi/WCNSS_qcom_cfg.ini:system/vendor/firmware/wlan/prima/WCNSS_qcom_cfg.ini \
    $(LOCAL_PATH)/wifi/WCNSS_qcom_wlan_nv.bin:system/vendor/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin

PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service.legacy \
    libwpa_client \
    hostapd \
    wificond \
    wpa_supplicant \
    wpa_supplicant.conf \
    wpa_supplicant_overlay.conf \
    p2p_supplicant_overlay.conf

# KeyLayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/atmel_mxt_ts.kl:system/vendor/usr/keylayout/atmel_mxt_ts.kl \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/vendor/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/keylayout/msm8974-taiko-mtp-snd-card_Button_Jack.kl:system/vendor/usr/keylayout/msm8974-taiko-mtp-snd-card_Button_Jack.kl \
    $(LOCAL_PATH)/keylayout/synaptics_rmi4_i2c.kl:system/vendor/usr/keylayout/synaptics_rmi4_i2c.kl

# SoftAP
PRODUCT_PACKAGES += \
    libwcnss_qmi \
    wcnss_service

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl \
    android.hardware.health@2.1-impl.recovery \
    android.hardware.health@2.1-service

# Gatekeeper
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-service.software

# IPC router config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:system/vendor/etc/sec_config

# NFC
PRODUCT_PACKAGES += \
    android.hardware.nfc@1.0-impl-bcm \
    nfc_nci.bcm2079x.default \
    Tag

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/vendor/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:system/vendor/etc/permissions/android.hardware.nfc.xml \
    $(LOCAL_PATH)/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nci.conf \
    $(LOCAL_PATH)/nfc/libnfc-nci-20795a10.conf:system/etc/libnfc-nci-20795a10.conf \
    $(LOCAL_PATH)/nfc/nfcee_access.xml:system/etc/nfcee_access.xml

# Proprietery Firmware
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/xtwifi.conf:system/etc/xtwifi.conf \
    $(LOCAL_PATH)/configs/calib.cfg:system/etc/calib.cfg \
    $(LOCAL_PATH)/configs/modem/Diag.cfg:system/etc/modem/Diag.cfg

# APEX
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ld.config.txt:system/etc/swcodec/ld.config.txt

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:system/vendor/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/audio_effects.xml:system/vendor/etc/audio_effects.xml \
    $(LOCAL_PATH)/audio/listen_platform_info.xml:system/vendor/etc/listen_platform_info.xml \
    $(LOCAL_PATH)/audio/diracmobile.config:system/vendor/etc/diracmobile.config \
    $(LOCAL_PATH)/audio/mixer_paths.xml:system/vendor/etc/mixer_paths.xml \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Bluetooth_cal.acdb:system/etc/acdbdata/MTP/MTP_Bluetooth_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_General_cal.acdb:system/etc/acdbdata/MTP/MTP_General_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Global_cal.acdb:system/etc/acdbdata/MTP/MTP_Global_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Handset_cal.acdb:system/etc/acdbdata/MTP/MTP_Handset_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Hdmi_cal.acdb:system/etc/acdbdata/MTP/MTP_Hdmi_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Headset_cal.acdb:system/etc/acdbdata/MTP/MTP_Headset_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/MTP/MTP_Speaker_cal.acdb:system/etc/acdbdata/MTP/MTP_Speaker_cal.acdb

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:system/vendor/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:system/vendor/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:system/vendor/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:system/vendor/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:system/vendor/etc/usb_audio_policy_configuration.xml

# Media profile
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs.xml:system/vendor/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media_profiles_V1_0.xml:system/vendor/etc/media_profiles_V1_0.xml \
    $(LOCAL_PATH)/configs/media_codecs_performance.xml:system/vendor/etc/media_codecs_performance.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/vendor/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/vendor/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/vendor/etc/media_codecs_google_video.xml

# Media & Audio
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service \
    libprotobuf-cpp-lite-v27 \
    libc2dcolorconvert \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxQcelp13Enc \
    libOmxVdec \
    libOmxVenc \
    libstagefrighthw \

PRODUCT_PACKAGES += \
    android.hardware.audio.service \
    android.hardware.audio@4.0-impl \
    android.hardware.audio.effect@4.0-impl \
    android.hardware.soundtrigger@2.0-impl \
    audio.primary.msm8974 \
    audio.r_submix.default \
    audio.usb.default \
    libaudio-resampler \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    tinymix

# Bluetooth HAL
PRODUCT_PACKAGES += \
    libbt-vendor \
    android.hardware.bluetooth@1.0-impl

# Filesystem management tools
PRODUCT_PACKAGES += \
    resize2fs \
    e2fsck_static \

# Graphics
PRODUCT_PACKAGES += \
    copybit.msm8974 \
    gralloc.msm8974 \
    hwcomposer.msm8974 \
    memtrack.msm8974 \
    liboverlay \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.memtrack-service.legacy

# RenderScript HAL
 PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Radio
PRODUCT_PACKAGES += \
    android.hardware.radio.config@1.0-service \
    libaudioclient_sfo_shim \
    libshim_powermanager \
    librmnetctl
    
# Seccomp policy
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:system/vendor/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/seccomp_policy/mediaextractor.policy:system/vendor/etc/seccomp_policy/mediaextractor.policy

# Sensors
PRODUCT_PACKAGES += \
    sensors.msm8974 \
    android.hardware.sensors@1.0-impl

# Thermal reporting; the proprietary thermal-engine retains hardware mitigation.
PRODUCT_PACKAGES += \
    android.hardware.thermal-service.sfo

# ANT+
PRODUCT_PACKAGES += \
    AntHalService_sfo \
    libantradio

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/vendor/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/vendor/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/vendor/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/vendor/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:system/vendor/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:system/vendor/etc/permissions/android.hardware.camera.raw.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/vendor/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:system/vendor/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/vendor/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/vendor/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/vendor/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/vendor/etc/permissions/android.hardware.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/vendor/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/vendor/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/vendor/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:system/vendor/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:system/vendor/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/vendor/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/vendor/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/vendor/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/vendor/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/vendor/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:system/vendor/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/vendor/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.software.midi.xml:system/vendor/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.print.xml:system/vendor/etc/permissions/android.software.print.xml

# Screen density
PRODUCT_AAPT_CONFIG := normal hdpi xhdpi xxhdpi
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

ifneq ($(QCPATH),)
$(call inherit-product-if-exists, $(QCPATH)/prebuilt_HY11/target/product/msm8974/prebuilt.mk)
endif

# call the proprietary setup
$(call inherit-product, vendor/smartisan/sfo/sfo-vendor.mk)
