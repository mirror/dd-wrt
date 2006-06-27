#$Id$

EXTRA_CFLAGS := -I$(TOPDIR)/arch/mips/bcm947xx/include -DBCMDRIVER

O_TARGET       := diag.o

MAC_OBJS       := diag_led.o

export-objs    := 
obj-y          := $(MAC_OBJS)
obj-m          := $(O_TARGET)

include $(TOPDIR)/Rules.make
