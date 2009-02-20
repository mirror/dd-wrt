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

ifeq ($(CONFIG_BCM5354),y)
export LINUXVER := 2.4.37
else
export LINUXVER := 2.4.35
endif

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
ifeq ($(CONFIG_SPEED),y)
export BASEOPT:=-O2
else
export BASEOPT:=-Os
endif


export ARCH:= mipsel
export ARCHITECTURE:= broadcom
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
export LDFLAGS := 

#
# Install and target directories
#

export PLATFORMDIR := $(TOP)/$(PLATFORM)
export INSTALLDIR := $(PLATFORMDIR)/install
export TARGETDIR := $(PLATFORMDIR)/target


#obj-y += libcrypto
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

all: clean_target $(obj-y) $(LINUXDIR)/.config kernel


kernel:
	# Also build kernel
ifeq ($(CONFIG_DIST),"micro")
	cp $(LINUXDIR)/.config_micro $(LINUXDIR)/.config
#	cd $(LINUXDIR) && make clean
else
ifeq ($(CONFIG_DIST),"micro-special")
	cp $(LINUXDIR)/.config_micro $(LINUXDIR)/.config
#	cd $(LINUXDIR) && make clean
else
ifeq ($(CONFIG_MADWIFI),y)
	cp $(LINUXDIR)/.config_atheros $(LINUXDIR)/.config
#	cd $(LINUXDIR) && make clean
else
	cp $(LINUXDIR)/.config_std $(LINUXDIR)/.config
#	cd $(LINUXDIR) && make clean
endif
endif
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
	echo CONFIG_NET_ETHERIP=m >> $(LINUXDIR)/.config
else
	echo "# CONFIG_NET_ETHERIP" is not set >> $(LINUXDIR)/.config
endif
ifeq ($(CONFIG_V25),y)
	echo CONFIG_V25=y >> $(LINUXDIR)/.config
else
	echo "# CONFIG_V25" is not set >> $(LINUXDIR)/.config
endif
#ifeq ($(CONFIG_REGISTER),y)
#	cp $(LINUXDIR)/.config_register $(LINUXDIR)/.config
#endif
	$(MAKE) -C $(LINUXDIR) oldconfig	
	if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) zImage ; \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) modules ; \
	fi
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(LINUXDIR)/vmlinux $(ARCH)-uclibc/vmlinus 
	cat $(ARCH)-uclibc/vmlinus | lzma e -si -so -eos -lc2 -lp2 -pb2 -d21 > lzma-loader/broadcom/vmlinux.lzma
	make -C lzma-loader/broadcom clean
	make -C lzma-loader/broadcom 
	dd if=lzma-loader/broadcom/lzma.elf of=$(ARCH)-uclibc/lzma_vmlinux bs=65536 conv=sync
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(ARCH)-uclibc/lzma_vmlinux $(ARCH)-uclibc/lzma_vmlinus 

bootloader:
	$(MAKE) -C $(LINUXDIR) oldconfig	
	if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) zImage ; \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) modules ; \
	fi
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(LINUXDIR)/vmlinux $(ARCH)-uclibc/vmlinus 
	cat $(ARCH)-uclibc/vmlinus | lzma e -si -so -eos -lc2 -lp2 -pb2 -d21 > lzma-loader/broadcom/vmlinux.lzma
	make -C lzma-loader/broadcom clean
	make -C lzma-loader/broadcom 
	cp lzma-loader/broadcom/lzma.elf $(ARCH)-uclibc/lzma_vmlinux
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(ARCH)-uclibc/lzma_vmlinux $(ARCH)-uclibc/lzma_vmlinus 

	
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
	if test -d "$(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/diag"; then mv $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/diag/* $(TARGETDIR)/lib/modules/$(LINUXVER); fi
#	mv $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/diag/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#ifneq ($(CONFIG_DIST),"micro")
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/led/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/switch/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#ifeq ($(CONFIG_MSSID),y)
#	cp ../wl/wl/wl_v24.o $(TARGETDIR)/lib/modules/2.4.33-rc1/wl.o
#else
	if test -d "$(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wl"; then mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wl/* $(TARGETDIR)/lib/modules/$(LINUXVER); fi              
ifeq ($(CONFIG_VXOVER),y)
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/wl.o
endif

ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	if test -d "$(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wireless"; then mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/wireless/* $(TARGETDIR)/lib/modules/$(LINUXVER); fi
endif
endif
ifeq ($(CONFIG_MSSID),y)
#ifneq ($(CONFIG_BCM5354),y)
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	cp bcm57xx/bcm57xx.o $(TARGETDIR)/lib/modules/$(LINUXVER)/bcm57xxlsys.o
#	cp bcm57xx/bcm57xx300.o $(TARGETDIR)/lib/modules/$(LINUXVER)/bcm57xx300.o
endif
endif
#endif
endif
#endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/fat/* $(TARGETDIR)/lib/modules/2.4.33-pre3
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	if test -d "$(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/jffs2"; then mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/jffs2/* $(TARGETDIR)/lib/modules/$(LINUXVER); fi
ifeq ($(CONFIG_SAMBA),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/cifs/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/ext2/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/ext2*
endif
ifeq ($(CONFIG_DIST),"mini")
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/cifs*
endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/vfat/* $(TARGETDIR)/lib/modules/2.4.33-prc1re3
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifneq ($(CONFIG_MSSID),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/lib/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
endif
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/bridge/netfilter/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv4/netfilter/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv4/* $(TARGETDIR)/lib/modules/$(LINUXVER)
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifeq ($(CONFIG_IPV6),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv6/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
endif
endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/net/imq.o $(TARGETDIR)/lib/modules/$(LINUXVER)
ifeq ($(CONFIG_MSSID),y)
ifeq ($(CONFIG_USB),y)
ifeq ($(CONFIG_USB_ADVANCED),y)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/ext3/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/jbd/* $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/fat/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/vfat/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/fs/xfs/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/cdrom/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/ide/pci/* $(TARGETDIR)/lib/modules/$(LINUXVER)
#	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/ide/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/scsi/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/host/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/storage/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/drivers/usb/* $(TARGETDIR)/lib/modules/$(LINUXVER)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/host
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/pci
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/storage
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/sr_mod.o
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/ide-scsi.o 
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
ifneq ($(CONFIG_JFFS2),y)
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/jffs2.o
endif
	rm -f $(TARGETDIR)/usr/lib/libpcap.so
	mipsel-linux-strip --strip-unneeded --remove-section=.comment $(TARGETDIR)/lib/modules/$(LINUXVER)/*.o		
ifeq ($(CONFIG_MMC),y)	
	cp ./mmc/mmc.o $(TARGETDIR)/lib/modules/$(LINUXVER)
endif
#	tar -cvf $(TARGETDIR)/lib/modules/modules.tar $(TARGETDIR)/lib/modules/$(LINUXVER)/* 
#	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)

        # Prepare filesystem
ifeq ($(CONFIG_DIST),"micro")
	cd $(TARGETDIR) && $(TOP)/misc/rootprep_micro.sh
else
ifeq ($(CONFIG_DIST),"micro-special")
	cd $(TARGETDIR) && $(TOP)/misc/rootprep_micro.sh
else
	cd $(TARGETDIR) && $(TOP)/misc/rootprep.sh
endif
endif
        # Make sure mksquashfs-2.0 is used
#	$(MAKE) -C $(LINUXDIR)/scripts/squashfs mksquashfs
	cd $(TARGETDIR)  &&  find . -iname "CVS" | xargs rm -rf
	cd $(TARGETDIR)  &&  find . -iname ".svn" | xargs rm -rf
ifeq ($(CONFIG_OPENSSL),y)
	rm -f $(TARGETDIR)/usr/sbin/openssl
endif
ifeq ($(CONFIG_KAID),y)
	make -C kaid/src
	cp kaid/kaid $(TARGETDIR)/usr/sbin
endif
	mv $(TARGETDIR)/usr/lib/services.so $(TARGETDIR)/lib
	cp $(LINUXDIR)/arch/mips/bcm947xx/compressed/piggy mipsel-uclibc/vmlinux

#	./tools/kernelopt/kernelopt.sh $(LINUXDIR) $(ARCH)-uclibc/target $(ARCH)-linux-uclibc-
#	make -C $(LINUXDIR) vmlinux_optimize
	
#	objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux_opt mipsel-uclibc/vmlinux
#	objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux mipsel-uclibc/vmlinux2
	../../opt/loader-0.02/lzma e -lc2 -lp2 -pb2 -d21 mipsel-uclibc/vmlinux mipsel-uclibc/vmlinuz
#	../../opt/loader-0.02/lzma e -lc2 -lp2 -pb2 -d21 mipsel-uclibc/vmlinux2 mipsel-uclibc/vmlinuz2
#	$(LINUXDIR)/scripts/squashfs/mksquashfs-lzma $(ARCH)-uclibc/target $(ARCH)-uclibc/target.squashfs -noappend -root-owned -le
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



