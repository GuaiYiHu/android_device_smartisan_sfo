# Copyright (C) 2014 The CyanogenMod Project
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

# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Mokee stuff.
$(call inherit-product, vendor/mokee/config/common_full_phone.mk)

# Inherit from sfo device
$(call inherit-product, device/smartisan/sfo/sfo.mk)

PRODUCT_NAME := mokee_sfo
PRODUCT_DEVICE := sfo
PRODUCT_BRAND := smartisan
PRODUCT_MANUFACTURER := smartisan
PRODUCT_MODEL := SM705

TARGET_VENDOR_DEVICE_NAME := msm8974sfo_lte
TARGET_VENDOR_PRODUCT_NAME := msm8974sfo_lte

PRODUCT_GMS_CLIENTID_BASE := android-smartisan

# Build fingerprint
BUILD_FINGERPRINT=smartisan/msm8974sfo_lte/msm8974sfo_lte:4.4.2/SANFRANCISCO:user/dev-keys

PRODUCT_BUILD_PROP_OVERRIDES += \
    TARGET_DEVICE="msm8974sfo_lte" \
    PRODUCT_NAME="msm8974sfo_lte" \
    PRIVATE_BUILD_DESC="msm8974sfo_lte-user 4.4.2 SANFRANCISCO dev-keys"
