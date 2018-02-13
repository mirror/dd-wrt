subdir-m += spl
subdir-m += splat

INSTALL_MOD_DIR ?= extra

SPL_MODULE_CFLAGS  = -I$(TOP)/spl/include
SPL_MODULE_CFLAGS += -include $(TOP)/spl/spl_config.h
export SPL_MODULE_CFLAGS

modules:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) CONFIG_SPL=m $@

clean:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) $@

install:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=extra modules_install
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules.*

all: modules
