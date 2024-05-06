#CC=cgcc

ifndef KERNEL_DIR
	KERNEL_DIR := /lib/modules/$(shell uname -r)/build
endif

EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DUNIT_TESTING

ccflags-y := -I$(src)/../../../src
ccflags-y += -I$(src)/..

MIN_REQS = ../../../src/common/types.o \
	../../../src/mod/common/types.o \
	../../../src/mod/common/address.o \
	../framework/unit_test.o

#../impersonator/stats.o
