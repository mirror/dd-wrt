VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(VERSION)
KBUILD := $(LINUXDIR)

CFLAGS_EXTRA += -I$(TOP)/antfs
CFLAGS_EXTRA += -I$(TOP)/antfs/include

CFLAGS_EXTRA += -DANTFS_LOGLEVEL_DEFAULT=ANTFS_LOGLEVEL_ERR
CFLAGS_EXTRA += -DCONFIG_ANTFS_SYMLINKS=y


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
	$(MAKE) -C $(KBUILD) M=$(CURDIR) modules $(MODULE_CONFIG) EXTRA_CFLAGS="$(CFLAGS_EXTRA)"
	$(call cmd_strip,$@)

clean: force
	$(MAKE) -C $(KBUILD) M=$(CURDIR) clean

.NOTPARALLEL:

install:
	install -v -m 644 -D antfs.ko $(IDIR)/antfs.ko


force: ;
