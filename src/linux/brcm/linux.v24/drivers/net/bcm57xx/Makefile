#
# Broadcom BCM57XX Gigabit Ethernet driver make file.
#
# $Id$
#

EXTRA_CFLAGS += -I$(TOPDIR)/arch/mips/bcm947xx/include

O_TARGET = bcm57xx.o

# If source directory src/bcm57xx exists, build objects from there

EXTRA_CFLAGS += -DDBG=0 -DT3_JUMBO_RCV_RCB_ENTRY_COUNT=256 -DNICE_SUPPORT -DBCMDRIVER
EXTRA_CFLAGS += -DPCIX_TARGET_WORKAROUND=1 -DINCLUDE_TBI_SUPPORT -DINCLUDE_5701_AX_FIX=1
EXTRA_CFLAGS += -I$(SRCBASE)/bcm57xx/linux -I$(SRCBASE)/bcm57xx/sys -I$(SRCBASE)/include



export-objs :=

obj-y := b57um.o b57proc.o b57diag.o tigon3.o autoneg.o 5701rls.o tcp_seg.o hndgige.o bcmrobo.o
obj-m := $(O_TARGET)


include $(TOPDIR)/Rules.make
