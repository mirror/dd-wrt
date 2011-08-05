bin_PROGRAMS = dlist_test get_device get_driver get_module
BUILT_SOURCES = test.h
CLEANFILES = test.h
test.h:
	./create-test
get_device_SOURCES = get_device.c
get_driver_SOURCES = get_driver.c
get_module_SOURCES = get_module.c
testlibsysfs_SOURCES = test.c test_attr.c test_bus.c test_class.c \
		       test_device.c test_driver.c test_module.c test_utils.c \
		       testout.c test-defs.h libsysfs.conf create-test
INCLUDES = -I../include
LDADD = ../lib/libsysfs.la
EXTRA_CFLAGS = @EXTRA_CLFAGS@
AM_CFLAGS = -Wall -W -Wstrict-prototypes $(EXTRA_CLFAGS)

