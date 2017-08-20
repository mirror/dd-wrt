LOCAL_PATH := $(call my-dir)

libext2_com_err_src_files := \
	error_message.c \
	et_name.c \
	init_et.c \
	com_err.c \
	com_right.c

libext2_com_err_c_includes := external/e2fsprogs/lib

libext2_com_err_cflags := -O2 -g -W -Wall

libext2_com_err_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_com_err_src_files)
LOCAL_C_INCLUDES := $(libext2_com_err_c_includes)
LOCAL_CFLAGS := $(libext2_com_err_cflags)
LOCAL_SYSTEM_SHARED_LIBRARIES := libc
LOCAL_MODULE := libext2_com_err
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_com_err_src_files)
LOCAL_C_INCLUDES := $(libext2_com_err_c_includes)
LOCAL_CFLAGS := $(libext2_com_err_cflags)
LOCAL_STATIC_LIBRARIES := libc
LOCAL_MODULE := libext2_com_err
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_com_err_src_files)
LOCAL_C_INCLUDES := $(libext2_com_err_c_includes)
LOCAL_CFLAGS := $(libext2_com_err_cflags)
LOCAL_MODULE := libext2_com_err-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)
