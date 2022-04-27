#
# Makefile for the TKIP regression test.
#
obj := $(firstword $(obj) $(SUBDIRS) .)
TOP = $(obj)/../..

obj-m			+= ath_test_tkip.o
ath_test_tkip-objs	:= test_tkip.o

include $(TOP)/Makefile.inc

INCS += -I$(TOP) -I$(WLAN)

EXTRA_CFLAGS+= $(INCS) $(COPTS)

-include $(TOPDIR)/Rules.make

all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

install:
	test -d $(DESTDIR)/$(KMODPATH) || mkdir -p $(DESTDIR)/$(KMODPATH)
	install -m 0644 ath_test_tkip.$(KMODSUF) $(DESTDIR)/$(KMODPATH)

clean:
	-rm -f *~ *.o *.ko *.mod.c
	-rm -f .depend .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd
	-rm -rf .tmp_versions

ath_test_tkip.o: $(ath_test_tkip-objs)
	$(LD) $(LDOPTS) -o ath_test_tkip.$(KMODSUF) -r $(ath_test_tkip-objs)
