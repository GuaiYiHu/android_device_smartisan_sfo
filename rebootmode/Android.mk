LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := sfo_reboot_mode
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := sfo_reboot_mode.c
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)
