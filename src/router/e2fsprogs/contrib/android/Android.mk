LOCAL_PATH := $(call my-dir)

#########################################################################
# Build statically linked e2fsdroid for recovery
e2fsdroid_src_files := \
        e2fsdroid.c \
        block_range.c \
        fsmap.c \
        block_list.c \
        base_fs.c \
        perms.c \
        basefs_allocator.c \
        hashmap.c \

e2fsdroid_cflags := -W -Wall -Werror -Wno-error=macro-redefined

e2fsdroid_static_libraries := \
        libext2_com_err \
        libext2_misc \
        libcutils \
        libselinux \
        libcrypto \
        libsparse \
        liblog \
        libz \

e2fsdroid_whole_static_libraries := \
        libbase \
        libext2fs \

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(e2fsdroid_src_files)
LOCAL_CFLAGS := $(e2fsdroid_cflags)
LOCAL_WHOLE_STATIC_LIBRARIES := $(e2fsdroid_whole_static_libraries)
LOCAL_STATIC_LIBRARIES := $(e2fsdroid_static_libraries)
LOCAL_MODULE := e2fsdroid_static
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

