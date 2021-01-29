VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)
KBUILD := $(LINUXDIR)



Q ?= @

MODULE_CONFIG += CONFIG_ANTFS_FS=m
MODULE_CONFIG += CONFIG_ANTFS_SYMLINKS=y

sources += $(wildcard include/*)
sources += $(wildcard libntfs-3g/*)
sources += $(wildcard *.h *.c)
sources += Makefile Kbuild Kconfig

destination = $(DESTDIR)/usr/src/$(DKMS_NAME)-$(VERSION)

default: antfs.ko

antfs.ko: force
	$(MAKE) -C $(KBUILD) M=$(CURDIR) modules $(MODULE_CONFIG)
	$(call cmd_strip,$@)

clean: force
	$(MAKE) -C $(KBUILD) M=$(CURDIR) clean

.NOTPARALLEL:

install:
	install -v -m 644 -D antfs.ko $(IDIR)/antfs.ko


force: ;
