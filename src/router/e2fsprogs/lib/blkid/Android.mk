LOCAL_PATH := $(call my-dir)

libext2_blkid_src_files := \
	cache.c \
	dev.c \
	devname.c \
	devno.c \
	getsize.c \
	llseek.c \
	probe.c \
	read.c \
	resolve.c \
	save.c \
	tag.c \
	version.c \


libext2_blkid_shared_libraries := libext2_uuid

libext2_blkid_system_shared_libraries := libc

libext2_blkid_static_libraries := libext2_uuid_static

libext2_blkid_system_static_libraries := libc

libext2_blkid_c_includes := external/e2fsprogs/lib

libext2_blkid_cflags := -O2 -g -W -Wall -fno-strict-aliasing

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_blkid_src_files)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_blkid_system_shared_libraries)
LOCAL_SHARED_LIBRARIES := $(libext2_blkid_shared_libraries)
LOCAL_C_INCLUDES := $(libext2_blkid_c_includes)
LOCAL_CFLAGS := $(libext2_blkid_cflags)
LOCAL_MODULE := libext2_blkid
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_blkid_src_files)
LOCAL_STATIC_LIBRARIES := $(libext2_blkid_static_libraries) $(libext2_blkid_system_static_libraries)
LOCAL_C_INCLUDES := $(libext2_blkid_c_includes)
LOCAL_CFLAGS := $(libext2_blkid_cflags) $(libext2_blkid_cflags_linux) -fno-strict-aliasing
LOCAL_MODULE := libext2_blkid
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_blkid_src_files)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(libext2_blkid_shared_libraries))
LOCAL_C_INCLUDES := $(libext2_blkid_c_includes)
LOCAL_CFLAGS := $(libext2_blkid_cflags)
LOCAL_MODULE := libext2_blkid-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)
