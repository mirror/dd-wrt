KERNEL       = ../../linux/linux.v24
INCLUDES     = -I/opt/openwrt/usr/include -I. -I$(KERNEL)/include -I$(KERNEL)/drivers/media/video -I../../include.v24

SOURCES      = adm6996.c
TARGETS      = $(SOURCES:.c=.o)

all:
	make -C $(KERNEL) SUBDIRS=`pwd` EXTRA_CFLAGS="$(INCLUDES) $(KCFLAGS)" modules
	
O_TARGET := adm6996.o

obj-y := adm_hw.o adm_procfs.o adm_init.o adm_ioctl.o adm_utils.o
obj-m := $(O_TARGET)

install:
	chmod 0644 $(O_TARGET)
	install -D $(O_TARGET) $(INSTALLDIR)/lib/modules/adm6996/$(O_TARGET)
# install -D $(O_TARGET) $(INSTALLDIR)/lib/modules/2.4.20/kernel/drivers/net/adm6996/$(O_TARGET)
	
clean:
	$(RM) $(O_TARGET) $(obj-y)
	$(RM) .*.flags


adm6996-objs := $(obj-y)

ifeq ($(PATCHLEVEL),4)
adm6996.o: $(adm6996-objs)
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -r -o adm6996.o $(adm6996-objs)
endif

-include $(TOPDIR)/Rules.make
