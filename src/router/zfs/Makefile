subdir-m += avl
subdir-m += nvpair
subdir-m += unicode
subdir-m += zcommon
subdir-m += zfs
subdir-m += zpios
subdir-m += icp

INSTALL_MOD_DIR ?= extra


ZFS_MODULE_CFLAGS += -include $(TOP)/spl/spl_config.h
ZFS_MODULE_CFLAGS += -include $(TOP)/zfs/zfs_config.h
ZFS_MODULE_CFLAGS += -I$(TOP)/zfs/tools/include -I$(TOP)/spl/include -I$(TOP)/spl
export ZFS_MODULE_CFLAGS

modules:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) CONFIG_ZFS=m $@

clean:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) $@

install:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=extra modules_install
	rm -f $(INSTALLDIR)/zfs/lib/modules/$(KERNELRELEASE)/modules.*

all: modules
