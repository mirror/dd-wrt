LOCAL_PATH := $(call my-dir)

###########################################################################
# Build fsstress
#
fsstress_src_files := \
	fsstress.c

fsstress_c_includes := 

fsstress_cflags := -O2 -g -W -Wall

fsstress_shared_libraries := 

fsstress_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(fsstress_src_files)
mke2fs_c_includesLOCAL_CFLAGS := $(fsstress_cflags)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(fsstress_system_shared_libraries)
LOCAL_MODULE := fsstress
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(fsstress_src_files)
LOCAL_CFLAGS := $(fsstress_cflags)
LOCAL_MODULE := fsstress_host
LOCAL_MODULE_STEM := fsstress
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_HOST_OS := linux

include $(BUILD_HOST_EXECUTABLE)

#########################################################################
# Build add_ext4_encrypt
#
include $(CLEAR_VARS)

add_ext4_encrypt_src_files := \
	add_ext4_encrypt.c

add_ext4_encrypt_c_includes := \
	external/e2fsprogs/lib

add_ext4_encrypt_cflags := -O2 -g -W -Wall

add_ext4_encrypt_shared_libraries := \
	libext2fs \
	libext2_com_err

add_ext4_encrypt_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(add_ext4_encrypt_src_files)
LOCAL_C_INCLUDES := $(add_ext4_encrypt_c_includes)
LOCAL_CFLAGS := $(add_ext4_encrypt_cflags)
LOCAL_SHARED_LIBRARIES := $(add_ext4_encrypt_shared_libraries)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(add_ext4_encrypt_system_shared_libraries)
LOCAL_MODULE := add_ext4_encrypt
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(add_ext4_encrypt_src_files)
LOCAL_C_INCLUDES := $(add_ext4_encrypt_c_includes)
LOCAL_CFLAGS := $(add_ext4_encrypt_cflags)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(add_ext4_encrypt_shared_libraries))
LOCAL_MODULE := add_ext4_encrypt_host
LOCAL_MODULE_STEM := add_ext4_encrypt
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)

