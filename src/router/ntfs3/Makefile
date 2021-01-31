VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)
KBUILD := $(LINUXDIR)

Q ?= @

MODULE_CONFIG += CONFIG_NTFS3_FS=m
MODULE_CONFIG += CONFIG_NTFS3_LZX_XPRESS=y

sources += $(wildcard lib/*)
sources += $(wildcard *.h *.c)
sources += Makefile Kbuild Kconfig

destination = $(DESTDIR)/usr/src/$(DKMS_NAME)-$(VERSION)

default: ntfs3

ntfs3.ko: force
	$(MAKE) -C $(KBUILD) M=$(CURDIR) modules $(MODULE_CONFIG) EXTRA_CFLAGS="$(CFLAGS_EXTRA)"
	$(call cmd_strip,$@)

clean: force
	$(MAKE) -C $(KBUILD) M=$(CURDIR) clean

.NOTPARALLEL:

install:
	install -v -m 644 -D ntfs3.ko $(IDIR)/ntfs3.ko


force: ;
