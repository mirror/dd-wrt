LOCAL_PATH := $(call my-dir)
IW_SOURCE_DIR := $(LOCAL_PATH)

include $(CLEAR_VARS)

IW_ANDROID_BUILD=y
NO_PKG_CONFIG=y
include $(LOCAL_PATH)/Makefile

LOCAL_SRC_FILES := $(patsubst %.o,%.c,$(OBJS))

LOCAL_CFLAGS += -DCONFIG_LIBNL20
LOCAL_LDFLAGS := -Wl,--no-gc-sections
#LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES := libnl
LOCAL_MODULE := iw

$(IW_SOURCE_DIR)/version.c:
	$(IW_SOURCE_DIR)/version.sh $(IW_SOURCE_DIR)/version.c

include $(BUILD_EXECUTABLE)
