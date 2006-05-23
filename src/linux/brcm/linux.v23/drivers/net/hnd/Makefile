#
# Makefile for the BCM47xx specific kernel interface routines
# under Linux.
#

EXTRA_CFLAGS	+= -I$(TOPDIR)/arch/mips/bcm947xx/include -DBCMDRIVER

O_TARGET	:= hnd.o

HND_OBJS	:= bcmutils.o hnddma.o linux_osl.o sbutils.o bcmsrom.o

export-objs	:= shared_ksyms.o
obj-y		:= shared_ksyms.o $(HND_OBJS)
obj-m           := $(O_TARGET)

include $(TOPDIR)/Rules.make

shared_ksyms.c: shared_ksyms.sh $(HND_OBJS)
	sh -e $< $(HND_OBJS) > $@
