#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

#EXTRA_CFLAGS := -DISPRINTK

SRCBASE		:= $(TOPDIR)/../..
EXTRA_CFLAGS	+= -I$(SRCBASE)/switch/sys -I$(SRCBASE)/include.v23


export-objs := br.o br_stp.o

O_TARGET	:= bridge.o
obj-y		:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_notify.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o

ifeq ($(CONFIG_NETFILTER),y)
export-objs += br_netfilter.o
obj-y		+= br_netfilter.o
endif

obj-m		:= $(O_TARGET)

include $(TOPDIR)/Rules.make
