# Broadcom Linux Router Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.2 2005/09/26 11:06:58 seg Exp $
#

include .config
ifneq ($(wildcard ../cy_conf.mak),)
  include ../cy_conf.mak
endif

#
# Paths
#
OPENSER_MODULES := sl tm rr maxfwd usrloc registrar dbtext textops exec auth auth_db nathelper
OPENSER_MODULE_FILES := $(foreach module,$(OPENSER_MODULES),openser/modules/$(module)/$(module).so)

# Source bases
export PLATFORM LINUXDIR LIBDIR USRLIBDIR
export TOP := $(shell pwd)
export SRCBASE := $(shell (cd $(TOP)/.. && pwd -P))
#
# Cross-compile environment variables
#

# Build platform
export BUILD := i386-pc-linux-gnu
export HOSTCC := gcc

# uClibc wrapper
ifeq ($(CONFIG_UCLIBC),y)
export PLATFORM := $(PLATFORM)-uclibc
endif

export LINUXVER := 2.4.34
ifneq ($(CONFIG_MSSID),y)
export LINUXDIR := $(SRCBASE)/linux/brcm/linux.v23
endif

ifeq ($(PLATFORM),mipsel)
export CROSS_COMPILE := mipsel-linux-
export CONFIGURE := ./configure mipsel-linux --build=$(BUILD)
export TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/../mipsel-linux && pwd -P)
endif

ifeq ($(PLATFORM),mipsel-uclibc)
export CROSS_COMPILE := mipsel-linux-uclibc-
export CONFIGURE := ./configure mipsel-linux --build=$(BUILD)
export TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/.. && pwd -P)
endif

ifeq ($(CONFIG_BCMWPA2),y)
export CFLAGS += -DBCMWPA2 
endif

ifeq ($(CONFIG_BRANDING),y)
export CFLAGS += -DCONFIG_BRANDING 
endif

export ARCH:= mipsel
export ARCHITECTURE:= broadcom
export BASEOPT:=-Os
export COPTS:=$(BASEOPT) -pipe -mips32 -mtune=mips32 -funit-at-a-time
export CC := $(CROSS_COMPILE)gcc
export AR := $(CROSS_COMPILE)ar
export AS := $(CROSS_COMPILE)as
export LD := $(CROSS_COMPILE)ld
export NM := $(CROSS_COMPILE)nm
export RANLIB := $(CROSS_COMPILE)ranlib
export STRIP := @true
export SIZE := $(CROSS_COMPILE)size

export CFLAGS :=$(BASEOPT) -pipe -mips32 -mtune=mips32 -funit-at-a-time
export LDFLAGS := -s

#
# Install and target directories
#

export PLATFORMDIR := $(TOP)/$(PLATFORM)
export INSTALLDIR := $(PLATFORMDIR)/install
export TARGETDIR := $(PLATFORMDIR)/target


obj-y += libcrypto
#
# Configuration
#
CONFIG_IPTABLES=y
#CONFIG_WLC=y
include rules/configs.mk

obj-clean := $(foreach obj,$(obj-y) $(obj-n),$(obj)-clean)
obj-install := $(foreach obj,$(obj-y),$(obj)-install)
obj-distclean := $(foreach obj,$(obj-y) $(obj-n),$(obj)-distclean)

#
# Basic rules
#

all: clean_target $(obj-y) $(LINUXDIR)/.config
        # Also build kernel
	# Also build kernel
	if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -C $(LINUXDIR) zImage ; \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -C $(LINUXDIR) modules ; \
	fi

	
clean: $(obj-clean)
	rm -f .config.old .config.cmd
	rm -rf $(INSTALLDIR)
	rm -rf $(TARGETDIR)
	rm -f $(ARCH)-uclibc/*
	rm -f $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.squashfs

clean_target:
	rm -rf $(TARGETDIR)
	rm -rf $(INSTALLDIR)
	rm -f $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.squashfs

distclean mrproper: $(obj-distclean) clean_target
	rm -rf $(INSTALLDIR)
	$(MAKE) -C $(LINUXDIR) distclean
	$(MAKE) -C $(LINUXDIR)/arch/mips/bcm947xx/compressed clean
	#rm -f $(PLATFORMDIR)/zImage $(PLATFORMDIR)/linux.bin
	$(MAKE) -C config clean
	rm -f .config $(LINUXDIR)/.config
	rm -f .config.old .config.cmd

optimize-lib:
#	../../tools/optimize_lib.sh netconf/ libnetconf.so libnetconf.a libnetconf_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libnetconf.so
	../../tools/optimize_lib.sh shared/ libshared.so libshared.a libshared_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libshared.so
#	../../tools/optimize_lib.sh lib/ libm.so libm.a libm_min.so $(TARGETDIR) $(TARGETDIR)/lib/libm.so.0
	../../tools/optimize_lib.sh lib/ libpthread.so libpthread.a libphread_min.so $(TARGETDIR) $(TARGETDIR)/lib/libpthread.so.0
#	../../tools/optimize_lib.sh services/ services.so services.a services_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/services.so
#  ../../tools/optimize_lib.sh iptables/ libiptables.so libiptables.a libiptables_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libiptables.so


install package: clean_target $(filter-out lib.mipsel-install,$(obj-install)) $(LINUXDIR)/.config
	install -d $(TARGETDIR)
	for dir in $(wildcard $(patsubst %,$(INSTALLDIR)/%,$(obj-y))) ; do \
	    (cd $${dir} && tar cpf - .) | (cd $(TARGETDIR) && tar xpf -) \
	done
	# optimize the crypto library by removing unneeded symbols
	#[ ! -d libcrypto ] || $(MAKE) -C libcrypto optimize
	#[ ! -d libcrypto ] || $(MAKE) -C libcrypto optimize
	# Install (and possibly optimize) C library
	$(MAKE) lib.mipsel-install
	# optimize the others library by removing unneeded symbols
	$(MAKE) optimize-lib
	# Install modules into filesystem
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -C $(LINUXDIR) modules_install DEPMOD=/bin/true INSTALL_MOD_PATH=$(TARGETDIR) ; \
	fi
ifeq ($(CONFIG_MADWIFI),y)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wl
endif
#	mkdir -p $(TARGETDIR)/lib/modules/2.4.32/kernel/drivers/net/ctmisc
#	cp ../wl/ctmisc/ctmisc.o $(TARGETDIR)/lib/modules/2.4.32/kernel/drivers/net/ctmisc
	
#	find $(TARGETDIR) -name "wl_*.o"  | xargs rm -rf
	# quick n dirty - stuff Nikki's module where it belongs
	#mkdir $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996
#if [ "$(CONFIG_MMC)" = "y" ] ; then \
#mkdir -p $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/mmc ; \
#cp mmc/mmc.o $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/mmc ; \
#fi ; 
#ifeq ($(CONFIG_MSSID),y)
#	cp adm6996.v24/adm6996.o $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996/adm6996.o
#else	
#	cp adm6996/adm6996.o $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996/adm6996.o
#endif

	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/build
	mv $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/diag/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#ifneq ($(CONFIG_DIST),"micro")
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/led/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/switch/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#ifeq ($(CONFIG_MSSID),y)
#	cp ../wl/wl/wl_v24.o $(TARGETDIR)/lib/modules/2.4.33-rc1/wl.o
#else
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wl/* $(TARGETDIR)/lib/modules/$(LINUXVER)
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wireless/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
ifeq ($(CONFIG_MSSID),y)
#	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/bcm57xx/*
	cp bcm57xx/bcm57xx.o $(TARGETDIR)/lib/modules/$(LINUXVER)/bcm57xxlsys.o
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
#endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/fat/* $(TARGETDIR)/lib/modules/2.4.33-pre3
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/jffs2/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/cifs/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/ext2/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
ifeq ($(CONFIG_DIST),"mini")
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/cifs*
endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/vfat/* $(TARGETDIR)/lib/modules/2.4.33-prc1re3
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_MSSID),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/lib/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/bridge/netfilter/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv4/netfilter/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv4/* $(TARGETDIR)/lib/modules/$(LINUXVER)
ifneq ($(CONFIG_DIST),"micro")
ifeq ($(CONFIG_IPV6),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv6/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_USB),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/ext3/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/jbd/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/cdrom/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/ide/pci/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/ide/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/scsi/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/host/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/storage/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/host
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/pci
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/storage
endif
endif
ifeq ($(CONFIG_PCMCIA),y)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/pcmcia
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/pcmcia/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/char/pcmcia/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/char/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/host/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/serial/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/host
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/serial
endif
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/pcmcia
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/netfilter
	rm -f $(TARGETDIR)/usr/lib/libpcap.so
	mipsel-linux-strip --strip-unneeded --remove-section=.comment $(TARGETDIR)/lib/modules/$(LINUXVER)/*.o		
        # Prepare filesystem
	cd $(TARGETDIR) && $(TOP)/misc/rootprep.sh
        # Make sure mksquashfs-2.0 is used
#	$(MAKE) -C $(LINUXDIR)/scripts/squashfs mksquashfs
	cd $(TARGETDIR)  &&  find . -iname "CVS" | xargs rm -rf
	cd $(TARGETDIR)  &&  find . -iname ".svn" | xargs rm -rf
ifeq ($(CONFIG_KAID),y)
	make -C kaid/src
	cp kaid/kaid $(TARGETDIR)/usr/sbin
endif
	mv $(TARGETDIR)/usr/lib/services.so $(TARGETDIR)/lib
	cp $(LINUXDIR)/arch/mips/bcm947xx/compressed/piggy mipsel-uclibc/vmlinux
	../../opt/loader-0.02/lzma e mipsel-uclibc/vmlinux mipsel-uclibc/vmlinuz
#endif
#
# Configuration rules
#
include rules/all.mk
#
# Generic rules
#

%:
	[ ! -d $* ] || $(MAKE) -C $* 

%-distclean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-clean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-install:
	[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(INSTALLDIR)/$*

$(obj-y) $(obj-n) $(obj-clean) $(obj-install): dummy

.PHONY: all clean distclean mrproper install package
.PHONY: conf mconf oldconf kconf kmconf config menuconfig oldconfig
.PHONY: dummy fixkc libnet libpcap



