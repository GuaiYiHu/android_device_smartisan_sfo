LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := libkkcomp.c
LOCAL_MODULE := libkkcomp
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_SHARED_LIBRARIES := libcrypto

include $(BUILD_SHARED_LIBRARY)
