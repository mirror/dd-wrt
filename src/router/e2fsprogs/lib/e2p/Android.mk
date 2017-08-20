LOCAL_PATH := $(call my-dir)

libext2_e2p_src_files := \
	feature.c \
	fgetflags.c \
	fsetflags.c \
	fgetproject.c \
	fsetproject.c \
	fgetversion.c \
	fsetversion.c \
	getflags.c \
	getversion.c \
	hashstr.c \
	iod.c \
	ls.c \
	mntopts.c \
	parse_num.c \
	pe.c \
	pf.c \
	ps.c \
	setflags.c \
	setversion.c \
	uuid.c \
	ostype.c \
	percent.c

libext2_e2p_c_includes := external/e2fsprogs/lib

libext2_e2p_cflags := -O2 -g -W -Wall

libext2_e2p_system_shared_libraries := libc

libext2_e2p_system_static_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_e2p_src_files)
LOCAL_C_INCLUDES := $(libext2_e2p_c_includes)
LOCAL_CFLAGS := $(libext2_e2p_cflags)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_e2p_system_shared_libraries)
LOCAL_MODULE := libext2_e2p
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_e2p_src_files)
LOCAL_C_INCLUDES := $(libext2_e2p_c_includes)
LOCAL_CFLAGS := $(libext2_e2p_cflags)
LOCAL_STATIC_LIBRARIES := $(libext2_e2p_system_static_libraries)
LOCAL_MODULE := libext2_e2p
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_e2p_src_files)
LOCAL_C_INCLUDES := $(libext2_e2p_c_includes)
LOCAL_CFLAGS := $(libext2_e2p_cflags)
LOCAL_MODULE := libext2_e2p-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)
