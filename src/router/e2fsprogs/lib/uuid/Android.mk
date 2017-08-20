LOCAL_PATH := $(call my-dir)

libext2_uuid_src_files := \
	clear.c \
	compare.c \
	copy.c \
	gen_uuid.c \
	isnull.c \
	pack.c \
	parse.c \
	unpack.c \
	unparse.c \
	uuid_time.c


libext2_uuid_c_includes := external/e2fsprogs/lib

libext2_uuid_cflags := -O2 -g -W -Wall \
	-Wno-unused-function \
	-Wno-unused-parameter

libext2_uuid_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_uuid_src_files)
LOCAL_C_INCLUDES := $(libext2_uuid_c_includes)
LOCAL_CFLAGS := $(libext2_uuid_cflags)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH) $(LOCAL_PATH)/..
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_uuid_system_shared_libraries)
LOCAL_MODULE := libext2_uuid
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_uuid_src_files)
LOCAL_C_INCLUDES := $(libext2_uuid_c_includes)
LOCAL_CFLAGS := $(libext2_uuid_cflags)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_MODULE := libext2_uuid-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_uuid_src_files)
LOCAL_C_INCLUDES := $(libext2_uuid_c_includes)
LOCAL_CFLAGS := $(libext2_uuid_cflags)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_STATIC_LIBRARIES := libc
LOCAL_MODULE := libext2_uuid_static
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_uuid_src_files)
LOCAL_C_INCLUDES := $(libext2_uuid_c_includes)
LOCAL_CFLAGS := $(libext2_uuid_cflags)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_MODULE := libext2_uuid-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_STATIC_LIBRARY)
