LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= -O3 -Dunix -DANDROID -DHAVE_ANSIC_C -DHAVE_PREAD -DNAME='"linux-arm"' -DLINUX_ARM -Dlinux -lrt -lpthread

LOCAL_SRC_FILES:= iozone.c libbif.c

LOCAL_MODULE:= iozone

LOCAL_CFLAGS +=-Wno-unused-parameter

include $(BUILD_EXECUTABLE)
