lib_LTLIBRARIES = libsysfs.la
libsysfs_la_SOURCES = sysfs_utils.c sysfs_attr.c sysfs_class.c dlist.c \
      sysfs_device.c sysfs_driver.c sysfs_bus.c sysfs_module.c sysfs.h
INCLUDES = -I../include
libsysfs_la_LDFLAGS = -version-info 2:1:0
EXTRA_CFLAGS = @EXTRA_CLFAGS@
libsysfs_la_CFLAGS = -Wall -W -Wstrict-prototypes $(EXTRA_CLFAGS)
