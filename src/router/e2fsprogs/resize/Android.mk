LOCAL_PATH := $(call my-dir)

resize2fs_src_files := \
	extent.c \
	resize2fs.c \
	main.c \
	online.c \
	sim_progress.c \
	resource_track.c

resize2fs_c_includes := external/e2fsprogs/lib

resize2fs_cflags := -O2 -g -W -Wall

resize2fs_shared_libraries := \
	libext2fs \
	libext2_com_err \
	libext2_e2p \
	libext2_uuid \
	libext2_blkid

resize2fs_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(resize2fs_src_files)
LOCAL_C_INCLUDES := $(resize2fs_c_includes)
LOCAL_CFLAGS := $(resize2fs_cflags)
LOCAL_SHARED_LIBRARIES := $(resize2fs_shared_libraries)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(resize2fs_system_shared_libraries)
LOCAL_MODULE := resize2fs
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(resize2fs_src_files)
LOCAL_C_INCLUDES := $(resize2fs_c_includes)
LOCAL_CFLAGS := $(resize2fs_cflags)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(resize2fs_shared_libraries))
LOCAL_MODULE := resize2fs_host
LOCAL_MODULE_STEM := resize2fs
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)
