KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
KDIR := $(LINUXDIR)
PWD := $(shell pwd)


#KERNEL_FLAGS = "-UCONFIG_NOPRINTK -I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch" 
#KERNEL_FLAGS=-I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1

all:
	$(MAKE) -C $(KDIR) M=$(PWD)/nat46 modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time"
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/common modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time"
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/nat64 modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time"
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/siit modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time"

install:
	$(MAKE) -C $(KDIR) M=$(PWD)/nat46 modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/common modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/nat64 modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/siit modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules*

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/nat46 clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/common clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/nat64 clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/jool/mod/siit clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -Wno-error=date-time" INSTALL_MOD_PATH=$(INSTALLDIR)
