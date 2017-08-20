LOCAL_PATH := $(call my-dir)

#########################
# Build the debugfs binary

debugfs_src_files :=  \
	debug_cmds.c \
	debugfs.c \
	util.c \
	ncheck.c\
	icheck.c \
	ls.c \
	lsdel.c \
	dump.c \
	set_fields.c \
	logdump.c \
	htree.c \
	unused.c \
	e2freefrag.c \
	filefrag.c \
	extent_cmds.c \
	extent_inode.c \
	zap.c \
	create_inode.c \
	quota.c \
	xattrs.c \
	journal.c \
	revoke.c \
	recovery.c \
	do_journal.c

debugfs_shared_libraries := \
	libext2fs \
	libext2_blkid \
	libext2_uuid \
	libext2_ss \
	libext2_quota \
	libext2_com_err \
	libext2_e2p

debugfs_system_shared_libraries := libc

debugfs_static_libraries := \
	libext2fs \
	libext2_blkid \
	libext2_uuid_static \
	libext2_ss \
	libext2_quota \
	libext2_com_err \
	libext2_e2p

debugfs_system_static_libraries := libc

debugfs_c_includes := \
	external/e2fsprogs/e2fsck \
	external/e2fsprogs/misc \
	external/e2fsprogs/lib

debugfs_cflags := -O2 -g -W -Wall -fno-strict-aliasing -DDEBUGFS

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(debugfs_src_files)
LOCAL_C_INCLUDES := $(debugfs_c_includes)
LOCAL_CFLAGS := $(debugfs_cflags)
LOCAL_SYSTEM_SHARED_LIBRARIES := $(debugfs_system_shared_libraries)
LOCAL_SHARED_LIBRARIES := $(debugfs_shared_libraries)
LOCAL_MODULE := debugfs
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(debugfs_src_files)
LOCAL_C_INCLUDES := $(debugfs_c_includes)
LOCAL_CFLAGS := $(debugfs_cflags)
LOCAL_STATIC_LIBRARIES := $(debugfs_static_libraries) $(debugfs_system_static_libraries)
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := debugfs_static
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(debugfs_src_files)
LOCAL_C_INCLUDES := $(debugfs_c_includes)
LOCAL_CFLAGS := $(debugfs_cflags)
LOCAL_SHARED_LIBRARIES := $(addsuffix -host, $(debugfs_shared_libraries))
LOCAL_MODULE := debugfs_host
LOCAL_MODULE_STEM := debugfs
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)
