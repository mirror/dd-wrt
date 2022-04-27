obj := $(firstword $(obj) $(SUBDIRS) .)
TOP = $(obj)/..

obj-y := ccmp/ tkip/ wep/

include $(TOP)/Makefile.inc

modules:
ifdef LINUX24
	for i in $(obj-y); do \
		$(MAKE) -C $$i || exit 1; \
	done
else
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules
endif

install:
	for i in $(obj-y); do \
		$(MAKE) -C $$i install || exit 1; \
	done

clean:
	for i in $(obj-y); do \
		$(MAKE) -C $$i clean; \
	done
