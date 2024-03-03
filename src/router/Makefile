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

ifneq ($(wildcard ../cy_conf.mak),)
  include ../cy_conf.mak
endif
include .config

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
export BASEOPT:=-Os -fno-unwind-tables -fno-asynchronous-unwind-tables -falign-jumps=1 -falign-labels=1 -falign-loops=1 -falign-functions=1 -fno-builtin-strlen -fno-guess-branch-probability -funsigned-char -finline-limit=0 -fno-builtin-printf
endif


export ARCH:= mipsel
export ARCHEXT: = .brcm24
export ARCHITECTURE:= broadcom
export COPTS:=$(BASEOPT) -pipe -mips32 -mtune=mips32 -funit-at-a-time  -mno-branch-likely -fno-caller-saves
export CC := ccache $(CROSS_COMPILE)gcc
export CXX := ccache $(CROSS_COMPILE)g++
export AR := $(CROSS_COMPILE)ar
export AS := $(CROSS_COMPILE)as
export LD := $(CROSS_COMPILE)ld
export NM := $(CROSS_COMPILE)nm
export RANLIB := $(CROSS_COMPILE)ranlib
export STRIP := @true
export SIZE := $(CROSS_COMPILE)size

export CFLAGS :=$(BASEOPT) -pipe -mips32 -mtune=mips32 -funit-at-a-time  -mno-branch-likely -fno-caller-saves
export LDFLAGS := 

#
# Install and target directories
#

export PLATFORMDIR := $(TOP)/$(PLATFORM)
export INSTALLDIR := $(PLATFORMDIR)/install
export TARGETDIR := $(PLATFORMDIR)/target


export LTO := -flto -fwhole-program
export LTOMIN := -flto
export LDLTO := -flto=jobserver -fuse-linker-plugin
export LTOPLUGIN := --plugin=$(shell $(CROSS_COMPILE)gcc --print-file-name=liblto_plugin.so)

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

include kernel-features.mk

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
	$(kernelfeatures)
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
	    $(MAKE) -C $(LINUXDIR) zImage ; \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -C $(LINUXDIR) modules ; \
	fi
	$(ARCH)-linux-objcopy -O binary $(LINUXDIR)/vmlinux $(ARCH)-uclibc/vmlinus 
	cat $(ARCH)-uclibc/vmlinus | lzma e -si -so -eos -lc2 -lp2 -pb2 -d25 > lzma-loader/broadcom/vmlinux.lzma
	make -C lzma-loader/broadcom clean
	make -C lzma-loader/broadcom 
	dd if=lzma-loader/broadcom/lzma.elf of=$(ARCH)-uclibc/lzma_vmlinux bs=65536 conv=sync
	$(ARCH)-linux-objcopy -O binary $(ARCH)-uclibc/lzma_vmlinux $(ARCH)-uclibc/lzma_vmlinus 

bootloader:
	$(MAKE) -C $(LINUXDIR) oldconfig	
	if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) zImage ; \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
	    $(MAKE) -j 4 -C $(LINUXDIR) modules ; \
	fi
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(LINUXDIR)/vmlinux $(ARCH)-uclibc/vmlinus 
	cat $(ARCH)-uclibc/vmlinus | lzma e -si -so -eos -lc2 -lp2 -pb2 -d25 > lzma-loader/broadcom/vmlinux.lzma
	make -C lzma-loader/broadcom clean
	make -C lzma-loader/broadcom 
	cp lzma-loader/broadcom/lzma.elf $(ARCH)-uclibc/lzma_vmlinux
	/opt/3.3.4/bin/$(ARCH)-linux-objcopy -O binary $(ARCH)-uclibc/lzma_vmlinux $(ARCH)-uclibc/lzma_vmlinus 

	
realclean: $(obj-clean)
	rm -f .config.old .config.cmd
	#umount $(TARGETDIR)
	rm -rf $(INSTALLDIR)
	rm -rf $(TARGETDIR)
	rm -f $(TARGETDIR)/*
	-rm -f $(ARCH)-uclibc/*

	
clean:  rc-clean httpd-clean services-clean shared-clean libutils-clean nvram-clean madwifi-clean madwifi_mimo-clean busybox-clean dnsmasq-clean iptables-clean pppd-clean iproute2-clean
	rm -f .config.old .config.cmd
	#umount $(TARGETDIR)
	rm -rf $(INSTALLDIR)
	rm -rf $(TARGETDIR)
	rm -f $(TARGETDIR)/*
	-rm -f $(ARCH)-uclibc/*

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
#	../../tools/optimize_lib.sh shared/ libshared.so libshared.a libshared_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libshared.so
ifeq ($(CONFIG_LIBM),y)
	../../tools/optimize_lib.sh lib.mipsel.brcm24/ libm.so libm.a libm_min.so $(TARGETDIR) $(TARGETDIR)/lib/libm.so.0
endif
ifeq ($(CONFIG_LIBPTHREAD),y)
	../../tools/optimize_lib.sh lib.mipsel.brcm24/ libpthread.so libpthread.a libpthread_min.so $(TARGETDIR) $(TARGETDIR)/lib/libpthread.so.0
endif
	../../tools/optimize_lib.sh libutils/ libutils.so libutils.a libutils_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libutils.so  $(TOP)/libutils/libutils_ld
	../../tools/optimize_lib.sh libutils/ libwireless.so libwireless.a libwireless_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libwireless.so $(TOP)/libutils/libwireless_ld
	../../tools/optimize_lib.sh libutils/ libshutils.so libshutils.a libshutils_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libshutils.so $(TOP)/libutils/libshutils_ld
	../../tools/optimize_lib.sh nvram/ libnvram.so libnvram.a libnvram_min.so $(TARGETDIR) $(TARGETDIR)/lib/libnvram.so $(TOP)/nvram/libnvram_ld
#	relink-lib.sh \
##		"$(ARCH)-linux-" \
#		"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}" \
#	"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_pic.a}" \
#		"$(ARCH)-uclibc/target/lib/libgcc_s.so.1" \
#		-Wl,--version-script=${shell $(ARCH)-linux-gcc -print-file-name=libgcc.map} -Wl,-soname=libgcc_s.so.1

#	../../tools/optimize_lib.sh services/ services.so services.a services_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/services.so
#  ../../tools/optimize_lib.sh iptables/ libiptables.so libiptables.a libiptables_min.so $(TARGETDIR) $(TARGETDIR)/usr/lib/libiptables.so

optimize-libnew:
ifneq ($(CONFIG_MUSL),y)
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libc.so.0} $(ARCH)-uclibc/target/lib/libc.so.0 
else
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libc.so} $(ARCH)-uclibc/target/lib/libc.so 
endif
ifneq ($(CONFIG_MUSL),y)
	cp ${shell $(ARCH)-linux-gcc -print-file-name=ld-uClibc.so.0} $(ARCH)-uclibc/target/lib/ld-uClibc.so.0 
else
	cd $(ARCH)-uclibc/target/lib && ln -sf libc.so $(MUSL_LD)
endif
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} $(ARCH)-uclibc/target/lib/libgcc_s.so.1 
ifeq ($(CONFIG_LIBDL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libdl.so.0} $(ARCH)-uclibc/target/lib/libdl.so.0 
endif
ifeq ($(CONFIG_LIBRT),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=librt.so.0} $(ARCH)-uclibc/target/lib/librt.so.0 
endif
ifeq ($(CONFIG_LIBNSL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libnsl.so.0} $(ARCH)-uclibc/target/lib/libnsl.so.0 
endif
ifeq ($(CONFIG_LIBUTIL),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libutil.so.0} $(ARCH)-uclibc/target/lib/libutil.so.0 
endif
ifeq ($(CONFIG_LIBCPP),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libstdc++.so.6} $(ARCH)-uclibc/target/lib/libstdc++.so.6 
endif
ifeq ($(CONFIG_LIBCRYPT),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libcrypt.so.0} $(ARCH)-uclibc/target/lib/libcrypt.so.0 
endif
ifeq ($(CONFIG_LIBM),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libm.so.0} $(ARCH)-uclibc/target/lib/libm.so.0 
endif
ifeq ($(CONFIG_LIBRESOLV),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libresolv.so.0} $(ARCH)-uclibc/target/lib/libresolv.so.0 
endif
ifeq ($(CONFIG_LIBPTHREAD),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libpthread.so.0} $(ARCH)-uclibc/target/lib/libpthread.so.0 
endif
ifeq ($(CONFIG_SQUID),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
ifeq ($(CONFIG_FREERADIUS),y)
	-cp ${shell $(ARCH)-linux-gcc -print-file-name=libatomic.so.1} $(ARCH)-uclibc/target/lib/libatomic.so.1 
endif
ifeq ($(CONFIG_RELINK),y)
ifneq ($(CONFIG_MUSL),y)
	relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc_so.a}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc_so.a}" \
		"$(ARCH)-uclibc/target/lib/libc.so.0" \
		-Wl,-init,__uClibc_init -Wl,-soname=libc.so.0 \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}
else
	relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc.a}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libc.a}" \
		"$(ARCH)-uclibc/target/lib/libc.so" \
		-Wl,-init,__uClibc_init -Wl,-soname=libc.so \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}
endif

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libcrypt.so}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libcrypt_pic.a}" \
		"$(ARCH)-uclibc/target/lib/libcrypt.so.0" \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libcrypt.so.0

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libm.so}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libm_pic.a}" \
		"$(ARCH)-uclibc/target/lib/libm.so.0" \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libm.so.0 

	-relink-lib.sh \
		"$(ARCH)-linux-" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libpthread.so.0}" \
		"${shell $(ARCH)-linux-gcc -print-file-name=libpthread_so.a}" \
		"$(ARCH)-uclibc/target/lib/libpthread.so.0" \
		-Wl,-z,nodelete,-z,initfirst,-init=__pthread_initialize_minimal_internal \
		${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} \
		-Wl,-soname=libpthread.so.0
endif
	cp ${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1} $(ARCH)-uclibc/target/lib/libgcc_s.so.1 
#	relink-lib.sh \
##		"$(ARCH)-linux-" \
#		"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_s.so.1}" \
#	"${shell $(ARCH)-linux-gcc -print-file-name=libgcc_pic.a}" \
#		"$(ARCH)-uclibc/target/lib/libgcc_s.so.1" \
#		-Wl,--version-script=${shell $(ARCH)-linux-gcc -print-file-name=libgcc.map} -Wl,-soname=libgcc_s.so.1



ifneq ($(CONFIG_NOOPT),y)
	rm -rf /tmp/$(ARCHITECTURE)/mklibs-out
	rm -f /tmp/$(ARCHITECTURE)/mklibs-progs
	-mkdir -p /tmp/$(ARCHITECTURE)/
	find $(TARGETDIR) -type f -perm /100 -exec \
		file -r -N -F '' {} + | \
		awk ' /executable.*dynamically/ { print $$1 }' > /tmp/$(ARCHITECTURE)/mklibs-progs

	find $(TARGETDIR) -type f -name \*.so\* -exec \
		file -r -N -F '' {} + | \
		awk ' /shared object/ { print $$1 }' >> /tmp/$(ARCHITECTURE)/mklibs-progs

	mkdir -p /tmp/$(ARCHITECTURE)/mklibs-out
ifneq ($(CONFIG_MUSL),y)
	mklibs.py -D \
		-d /tmp/$(ARCHITECTURE)/mklibs-out \
		--sysroot $(TARGETDIR) \
		-L /lib \
		-L /usr/lib \
		--ldlib /lib/ld-uClibc.so.0 \
		--target $(ARCH)-linux-uclibc \
		`cat /tmp/$(ARCHITECTURE)/mklibs-progs` 2>&1
else
	mklibs.py -D \
		-d /tmp/$(ARCHITECTURE)/mklibs-out \
		--sysroot $(TARGETDIR) \
		-L /lib \
		-L /usr/lib \
		--ldlib /lib/$(MUSL_LD) \
		--target $(ARCH)-linux-uclibc \
		`cat /tmp/$(ARCHITECTURE)/mklibs-progs` 2>&1

endif
	cp /tmp/$(ARCHITECTURE)/mklibs-out/* $(TARGETDIR)/lib
endif


install package: clean_target $(filter-out lib.mipsel.brcm24-install,$(obj-install)) $(LINUXDIR)/.config
	install -d $(TARGETDIR)
	for dir in $(wildcard $(patsubst %,$(INSTALLDIR)/%,$(obj-y))) ; do \
	    (cd $${dir} && tar cpf - .) | (cd $(TARGETDIR) && tar xpf -) \
	done
	# optimize the crypto library by removing unneeded symbols
	#[ ! -d libcrypto ] || $(MAKE) -C libcrypto optimize
	#[ ! -d libcrypto ] || $(MAKE) -C libcrypto optimize
	# Install (and possibly optimize) C library
ifeq ($(CONFIG_OPENSSL),y)
	rm -f $(TARGETDIR)/usr/sbin/openssl
endif
	$(MAKE) lib.mipsel.brcm24-install
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
ifneq ($(CONFIG_MMC),y)
	rm -f $(TARGETDIR)/lib/modules/$(LINUXVER)/ext2*
endif
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
	mv -f $(TARGETDIR)/lib/modules/$(LINUXVER)/kernel/net/ipv6/netfilter/* $(TARGETDIR)/lib/modules/$(LINUXVER)
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
ifneq ($(CONFIG_3G),y)
	rm -rf $(TARGETDIR)/lib/modules/$(LINUXVER)/serial
endif
	mipsel-linux-strip --strip-unneeded --remove-section=.comment $(TARGETDIR)/lib/modules/$(LINUXVER)/*.o		
ifeq ($(CONFIG_MMC),y)	
	cp ./mmc_sdhc/mmc.o $(TARGETDIR)/lib/modules/$(LINUXVER)
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
#ifeq ($(CONFIG_KAID),y)
#	make -C kaid/src
#	cp kaid/kaid $(TARGETDIR)/usr/sbin
#endif
#	mv $(TARGETDIR)/usr/lib/services.so $(TARGETDIR)/lib

	$(TOP)/../../tools/removewhitespace.sh $(TOP)/../../tools $(TARGETDIR)/etc/config

	cp $(LINUXDIR)/arch/mips/bcm947xx/compressed/piggy mipsel-uclibc/vmlinux

#	./tools/kernelopt/kernelopt.sh $(LINUXDIR) $(ARCH)-uclibc/target $(ARCH)-linux-uclibc-
#	make -C $(LINUXDIR) vmlinux_optimize
	
#	objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux_opt mipsel-uclibc/vmlinux
#	objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux mipsel-uclibc/vmlinux2
	../../opt/loader-0.02/lzma e -lc2 -lp2 -pb2 -d25 mipsel-uclibc/vmlinux mipsel-uclibc/vmlinuz
#	../../opt/loader-0.02/lzma e -lc2 -lp2 -pb2 -d21 mipsel-uclibc/vmlinux2 mipsel-uclibc/vmlinuz2
#	$(LINUXDIR)/scripts/squashfs/mksquashfs-lzma $(ARCH)-uclibc/target $(ARCH)-uclibc/target.squashfs -noappend -root-owned -le
#endif
#
# Configuration rules
#
include rules/all.mk

.PHONY: all clean distclean mrproper install package
.PHONY: conf mconf oldconf kconf kmconf config menuconfig oldconfig
.PHONY: dummy fixkc libnet libpcap



