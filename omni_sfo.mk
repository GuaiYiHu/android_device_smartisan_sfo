$(call inherit-product, build/target/product/embedded.mk)
$(call inherit-product, vendor/omni/config/common.mk)

PRODUCT_PACKAGES += \
    sfo_reboot_mode

PRODUCT_NAME := omni_sfo
PRODUCT_DEVICE := sfo
PRODUCT_BRAND := Smartisan
PRODUCT_MODEL := Smartisan T1
PRODUCT_MANUFACTURER := smartisan
PRODUCT_RELEASE_NAME := sfo
