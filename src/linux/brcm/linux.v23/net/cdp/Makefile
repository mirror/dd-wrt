#
# Makefile for the Linux CDP layer.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

# We only get in/to here if CONFIG_CDP = 'y' or 'm'

O_TARGET := cdp.o

export-objs = af_cdp.o

obj-y	:= af_cdp.o

ifeq ($(CONFIG_CDP),m)
  obj-m += $(O_TARGET)
endif

include $(TOPDIR)/Rules.make

tar:
		tar -cvf /dev/f1 .
