LOCAL_PATH := $(call my-dir)

libext2_ss_src_files := \
	ss_err.c \
	std_rqs.c \
	invocation.c help.c \
	execute_cmd.c \
	listen.c \
	parse.c \
	error.c \
	prompt.c \
	request_tbl.c \
	list_rqs.c \
	pager.c \
	requests.c \
	data.c \
	get_readline.c

libext2_ss_c_includes := external/e2fsprogs/lib

libext2_ss_cflags := -O2 -g -W -Wall

libext2_ss_shared_libraries := \
	libext2_com_err

libext2_ss_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_ss_src_files)
LOCAL_C_INCLUDES := $(libext2_ss_c_includes)
LOCAL_CFLAGS := $(libext2_ss_cflags)
LOCAL_SHARED_LIBRARIES := $(libext2_ss_shared_libraries)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(libext2_ss_system_shared_libraries)
LOCAL_MODULE := libext2_ss
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_ss_src_files)
LOCAL_C_INCLUDES := $(libext2_ss_c_includes)
LOCAL_CFLAGS := $(libext2_ss_cflags)
LOCAL_STATIC_LIBRARIES := libc
LOCAL_MODULE := libext2_ss
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libext2_ss_src_files)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(libext2_ss_shared_libraries))
LOCAL_C_INCLUDES := $(libext2_ss_c_includes)
LOCAL_CFLAGS := $(libext2_ss_cflags)
LOCAL_MODULE := libext2_ss-host
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_SHARED_LIBRARY)
