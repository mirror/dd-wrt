LOCAL_PATH := $(call my-dir)

libext2_quota_src_files := \
	dict.c \
	mkquota.c \
	parse_qtype.c \
	plausible.c \
	profile.c \
	profile_helpers.c \
	prof_err.c \
	quotaio.c \
	quotaio_tree.c \
	quotaio_v2.c

libext2_quota_c_includes := external/e2fsprogs/lib

libext2_quota_cflags := -O2 -g -W -Wall

libext2_quota_shared_libraries := libext2fs libext2_com_err libext2_blkid

libext2_quota_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_quota_src_files)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_quota_system_shared_libraries)
LOCAL_C_INCLUDES := $(libext2_quota_c_includes)
LOCAL_CFLAGS := $(libext2_quota_cflags)
LOCAL_SYSTEM_SHARED_LIBRARIES := libc $(libext2_quota_shared_libraries)
LOCAL_MODULE := libext2_quota
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

libext2_quota_static_libraries := libext2fs libext2_com_err

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_quota_src_files)
LOCAL_C_INCLUDES := $(libext2_quota_c_includes)
LOCAL_CFLAGS := $(libext2_quota_cflags)
LOCAL_STATIC_LIBRARIES := libc $(libext2_quota_static_libraries)
LOCAL_MODULE := libext2_quota
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_quota_src_files)
LOCAL_C_INCLUDES := $(libext2_quota_c_includes)
LOCAL_CFLAGS := $(libext2_quota_cflags)
LOCAL_MODULE := libext2_quota-host
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(libext2_quota_shared_libraries))

include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)

libext2_profile_src_files :=  \
       prof_err.c \
       profile.c

libext2_profile_shared_libraries := \
       libext2_com_err

libext2_profile_system_shared_libraries := libc

libext2_profile_c_includes := external/e2fsprogs/lib

libext2_profile_cflags := -O2 -g -W -Wall

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_profile_src_files)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_profile_system_shared_libraries)
LOCAL_SHARED_LIBRARIES := $(libext2_profile_shared_libraries)
LOCAL_C_INCLUDES := $(libext2_profile_c_includes)
LOCAL_CFLAGS := $(libext2_profile_cflags)
LOCAL_MODULE := libext2_profile
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_profile_src_files)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(libext2_profile_shared_libraries))
LOCAL_C_INCLUDES := $(libext2_profile_c_includes)
LOCAL_CFLAGS := $(libext2_profile_cflags)
LOCAL_MODULE := libext2_profile-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)
