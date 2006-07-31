#
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
SER_MODULES:=sl tm rr maxfwd usrloc registrar dbtext textops nathelper
SER_MODULE_FILES:=$(foreach module,$(SER_MODULES),ser/modules/$(module)/$(module).so)

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

ifeq ($(CONFIG_MSSID),y)
export LINUXDIR := $(SRCBASE)/linux/brcm/linux.v24
else
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

export ARCH:= mipsel
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
obj-$(CONFIG_OPENSSL) += openssl
obj-$(CONFIG_MATRIXSSL) += matrixssl
obj-$(CONFIG_HTTPD) += httpd validate
obj-$(CONFIG_RC) += rc services
#obj-$(CONFIG_LIBBCM) += libbcm
obj-$(CONFIG_SHARED) += shared
obj-$(CONFIG_WWW) += www
obj-$(CONFIG_GLIBC) += lib.mipsel
obj-$(CONFIG_UCLIBC) += lib.mipsel
obj-$(CONFIG_BUSYBOX) += busybox net-tools
obj-$(CONFIG_DNSMASQ) += dnsmasq
obj-$(CONFIG_IPTABLES) += iptables
obj-$(CONFIG_LIBIPT) += iptables
obj-$(CONFIG_IPSEC) += ipsec
obj-$(CONFIG_LIBPCAP) += libpcap
obj-$(CONFIG_LIBNET) += net
obj-$(CONFIG_TCPDUMP) += tcpdump
obj-$(CONFIG_KISMETDRONE) += kismet
obj-$(CONFIG_NETSTATNAT) += netstatnat

ifneq ($(CONFIG_MADWIFI),y)
obj-$(CONFIG_UTILS) += utils
obj-$(CONFIG_NAS) += nas
obj-$(CONFIG_WLCONF) += wlconf
endif
obj-$(CONFIG_SES) += ses
obj-$(CONFIG_WPA_SUPPLICANT) += wpa_supplicant
obj-$(CONFIG_NETCONF) += netconf
obj-$(CONFIG_NTP) += ntpclient
obj-$(CONFIG_NVRAM) += nvram
obj-$(CONFIG_PPP) += ppp
# AhMan March 19 2005
obj-$(CONFIG_PPPOE) += pppoe
obj-$(CONFIG_UDHCPD) += udhcpd
obj-$(CONFIG_UPNP) += upnp
obj-$(CONFIG_MADWIFI) += madwifi
obj-$(CONFIG_ETC) += etc
#obj-$(CONFIG_VLAN) += vlan
obj-$(CONFIG_WIRELESS_TOOLS) += wireless-tools
obj-$(CONFIG_DROPBEAR_SSHD) += zlib dropbear
obj-$(CONFIG_IPROUTE2) += iproute2
obj-$(CONFIG_EBTABLES) += ebtables
obj-$(CONFIG_PPTPD) += pptpd
obj-$(CONFIG_PIPSEC) += pipsec
obj-$(CONFIG_FROTTLE) += frottle
obj-$(CONFIG_WOL) += wol
obj-$(CONFIG_SNMP) += snmp
obj-$(CONFIG_RADVD) += radvd
obj-$(CONFIG_RFLOW) += rflow
#obj-$(CONFIG_ADM6996) += adm6996
obj-$(CONFIG_SPUTNIK_APD) += sputnik

##################################################################
CONFIG_OTHERS=y
#obj-$(CONFIG_ADM6996) += adm6996
obj-$(CONFIG_L2TP) += rp-l2tp

obj-$(CONFIG_CHILLISPOT) += chillispot
obj-$(CONFIG_DHCPFORWARD) += dhcpforwarder
obj-$(CONFIG_PARPROUTED) += parprouted
obj-$(CONFIG_HEARTBEAT) += bpalogin
#obj-$(CONFIG_TFTPD) += tftpd
obj-$(CONFIG_CRON) += cron
obj-$(CONFIG_PPTP) += pptp-client
obj-$(CONFIG_PPPD) += pppd
obj-$(CONFIG_ZEBRA) += zebra
obj-$(CONFIG_BIRD) += bird
obj-$(CONFIG_DDNS) += inadyn
obj-$(CONFIG_OTHERS) += others
obj-$(CONFIG_EOU) += eou
obj-$(CONFIG_SER) += ser
obj-$(CONFIG_NOCAT) += nocat
obj-$(CONFIG_RTPPROXY) += rtpproxy
obj-$(CONFIG_SAMBA) += samba
obj-$(CONFIG_RADAUTH) += radauth
obj-$(CONFIG_MMC) += mmc
obj-$(CONFIG_ZEROIP) += shat
obj-$(CONFIG_KAID) += kaid
#obj-$(CONFIG_ROBOCFG) += robocfg
obj-$(CONFIG_MULTICAST) += igmp-proxy
obj-$(CONFIG_SKYTRON) += skytron
obj-$(CONFIG_OPENVPN) += openvpn
obj-$(CONFIG_OLSRD) += olsrd
#obj-$(CONFIG_E2FSPROGS) += e2fsprogs
obj-$(CONFIG_HTTPREDIRECT) += http-redirect
obj-$(CONFIG_SMTPREDIRECT) += smtp-redirect
obj-$(CONFIG_WLC) += wlc
obj-$(CONFIG_OVERCLOCKING) += overclocking
obj-$(CONFIG_PROXYWATCHDOG) += proxywatchdog
obj-$(CONFIG_JFFS2) += jffs2
obj-$(CONFIG_LANGUAGE) += language
obj-$(CONFIG_NETWORKSETTINGS) += networksettings
obj-$(CONFIG_ROUTERSTYLE) += routerstyle
obj-$(CONFIG_SCHEDULER) += scheduler
obj-$(CONFIG_SYSLOG) += syslog
obj-$(CONFIG_TELNET) += telnet
obj-$(CONFIG_WDSWATCHDOG) += wdswatchdog
obj-$(CONFIG_IPV6) += ipv6
obj-$(CONFIG_CONNTRACK) += conntrack
obj-$(CONFIG_MEDIASERVER) += mediaserver
obj-$(CONFIG_RADIOOFF) += radiooff
obj-y += sprom



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
	rm -f $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.squashfs

clean_target:
	rm -rf $(TARGETDIR)
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


install package: $(filter-out lib.mipsel-install,$(obj-install)) $(LINUXDIR)/.config
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
	rm -rf $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/drivers/net/wl
endif
#	mkdir -p $(TARGETDIR)/lib/modules/2.4.32/kernel/drivers/net/ctmisc
#	cp ../wl/ctmisc/ctmisc.o $(TARGETDIR)/lib/modules/2.4.32/kernel/drivers/net/ctmisc
	
#	find $(TARGETDIR) -name "wl_*.o"  | xargs rm -rf
	# quick n dirty - stuff Nikki's module where it belongs
	#mkdir $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996
	if [ "$(CONFIG_MMC)" = "y" ] ; then \
	mkdir -p $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/fs/mmc ; \
	cp mmc/mmc.o $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/fs/mmc ; \
	fi ; 
#ifeq ($(CONFIG_MSSID),y)
#	cp adm6996.v24/adm6996.o $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996/adm6996.o
#else	
#	cp adm6996/adm6996.o $(TARGETDIR)/lib/modules/2.4.32-rc3/kernel/drivers/net/adm6996/adm6996.o
#endif

	rm -f $(TARGETDIR)/lib/modules/2.4.33-rc2/build
	mv $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/drivers/net/diag/* $(TARGETDIR)/lib/modules/2.4.33-rc2
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/drivers/net/led/* $(TARGETDIR)/lib/modules/2.4.33-rc2
endif
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/drivers/net/switch/* $(TARGETDIR)/lib/modules/2.4.33-rc2
#ifeq ($(CONFIG_MSSID),y)
#	cp ../wl/wl/wl_v24.o $(TARGETDIR)/lib/modules/2.4.33-rc1/wl.o
#else
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/drivers/net/wl/* $(TARGETDIR)/lib/modules/2.4.33-rc2
#endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/fat/* $(TARGETDIR)/lib/modules/2.4.33-pre3
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/fs/jffs2/* $(TARGETDIR)/lib/modules/2.4.33-rc2
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/fs/smbfs/* $(TARGETDIR)/lib/modules/2.4.33-rc2
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/fs/ext2/* $(TARGETDIR)/lib/modules/2.4.33-rc2
endif
#	mv $(TARGETDIR)/lib/modules/2.4.33-pre3/kernel/fs/vfat/* $(TARGETDIR)/lib/modules/2.4.33-prc1re3
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_MSSID),y)
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/lib/* $(TARGETDIR)/lib/modules/2.4.33-rc2
endif
endif
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/net/bridge/netfilter/* $(TARGETDIR)/lib/modules/2.4.33-rc2
endif
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/net/ipv4/netfilter/* $(TARGETDIR)/lib/modules/2.4.33-rc2
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/net/ipv4/* $(TARGETDIR)/lib/modules/2.4.33-rc2
ifneq ($(CONFIG_DIST),"micro")
	mv -f $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel/net/ipv6/* $(TARGETDIR)/lib/modules/2.4.33-rc2
endif
	rm -rf $(TARGETDIR)/lib/modules/2.4.33-rc2/kernel
	rm -rf $(TARGETDIR)/lib/modules/2.4.33-rc2/pcmcia
	rm -rf $(TARGETDIR)/lib/modules/2.4.33-rc2/netfilter
	mipsel-linux-strip --strip-unneeded --remove-section=.comment $(TARGETDIR)/lib/modules/2.4.33-rc2/*.o		
        # Prepare filesystem
	cd $(TARGETDIR) && $(TOP)/misc/rootprep.sh
        # Make sure mksquashfs-2.0 is used
	$(MAKE) -C $(LINUXDIR)/scripts/squashfs mksquashfs
	cd $(TARGETDIR)  &&  find . -iname "CVS" | xargs rm -rf
	cd $(TARGETDIR)  &&  find . -iname ".svn" | xargs rm -rf
ifeq ($(CONFIG_KAID),y)
	cp kaid/kaid $(TARGETDIR)/usr/sbin
endif
	cp $(LINUXDIR)/arch/mips/bcm947xx/compressed/piggy mipsel-uclibc/vmlinux
	../../opt/loader-0.02/lzma e mipsel-uclibc/vmlinux mipsel-uclibc/vmlinuz
	
#endif
#
# Configuration rules
#
fixkc: $(LINUXDIR)/.config
	cp $(LINUXDIR)/.config /tmp/config.$$$$ ; \
  if [ "$(CONFIG_IPSEC_TUNNEL)" = "y" ] ; then \
    sed -e '/^CONFIG_IPSEC_TUNNEL=.*$$/d' \
				-e '/^CONFIG_CRYPTO=.*$$/d' \
        -e '/^CONFIG_CIPHERS=.*$$/d' \
        -e '/^CONFIG_DIGESTS=.*$$/d' \
        -e '/^CONFIG_CIPHER_DES=.*$$/d' \
        -e '/^CONFIG_CIPHER_3DES=.*$$/d' \
        -e '/^CONFIG_CIPHER_AES=.*$$/d' \
        -e '/^CONFIG_DIGEST_MD5=.*$$/d' \
        -e '/^CONFIG_DIGEST_SHA1=.*$$/d' \
        /tmp/config.$$$$ > /tmp/config.n.$$$$ ; \
    mv -f /tmp/config.n.$$$$ /tmp/config.$$$$ ; \
    echo 'CONFIG_IPSEC_TUNNEL=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_CRYPTO=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_DIGESTS=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_CIPHERS=y' >> /tmp/config.$$$$ ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_MD5)" = "y" ] ; then \
      echo 'CONFIG_DIGEST_MD5=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_SHA1)" = "y" ] ; then \
      echo 'CONFIG_DIGEST_SHA1=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_DES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_DES=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_3DES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_3DES=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_AES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_AES=y' >> /tmp/config.$$$$ ; \
    fi ; \
  fi && mv /tmp/config.$$$$ $(LINUXDIR)/.config


conf mconf:
	$(MAKE) -C config
	@./config/$@ ./config/Config
	$(MAKE) fixkc
        # Also configure kernel
	$(MAKE) k$@

oldconf: .config
	$(MAKE) -C config
	@./config/conf -o ./config/Config
	$(MAKE) fixkc
        # Also configure kernel
	$(MAKE) k$@

kconf:
	$(MAKE) -C $(LINUXDIR) config

kmconf:
	$(MAKE) -C $(LINUXDIR) menuconfig

koldconf: $(LINUXDIR)/.config
	$(MAKE) -C $(LINUXDIR) oldconfig

# Convenience
config: conf

menuconfig: mconf

oldconfig: oldconf

# Default configurations
.config:
	cp config/defconfig $@
	$(MAKE) oldconfig

$(LINUXDIR)/.config:
	cp $(LINUXDIR)/arch/mips/defconfig-bcm947xx $@

#
# Overrides
#


matrixssl:
	$(MAKE) -C matrixssl/src all

matrixssl-install:
	echo "nischt"
#	install -D matrixssl/src/libmatrixssl.so $(INSTALLDIR)/matrixssl/usr/lib/libmatrixssl.so
#	$(STRIP) $(INSTALLDIR)/matrixssl/usr/lib/libmatrixssl.so

matrixssl-clean:
	$(MAKE) -C matrixssl/src clean


openssl:
	$(MAKE) -C openssl

openssl-install:
	install -D openssl/libcrypto.so $(INSTALLDIR)/openssl/usr/lib/libcrypto.so
	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libcrypto.so
	install -D openssl/libssl.so $(INSTALLDIR)/openssl/usr/lib/libssl.so
	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libssl.so

openssl-clean:
	$(MAKE) -C openssl clean

rp-l2tp-install:
	install -d $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp
	install rp-l2tp/handlers/*.so $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/lib/l2tp/*.so
	install -D rp-l2tp/handlers/l2tp-control $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tp-control
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tp-control
	install -D rp-l2tp/l2tpd $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tpd
	$(STRIP) $(INSTALLDIR)/rp-l2tp/usr/sbin/l2tpd

libpcap:
	$(MAKE) -C libpcap CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.so

libpcap-install:
#	install -d $(INSTALLDIR)/libpcap/usr/lib
#	install libpcap/libpcap.so $(INSTALLDIR)/libpcap/usr/lib
#	$(STRIP) $(INSTALLDIR)/libpcap/usr/lib/libpcap.so

eou: libnet libpcap
	[ ! -d eou ] || $(MAKE) -C eou

eou-install:
	[ ! -d eou ] || install -D eou/eou $(INSTALLDIR)/eou/usr/sbin/eou
	$(STRIP) $(INSTALLDIR)/eou/usr/sbin/eou

ses:
	[ ! -f ses/Makefile ] || $(MAKE) -C ses

ses-install:
	install -D ses/ses/ses $(INSTALLDIR)/ses/usr/sbin/ses
	$(STRIP) $(INSTALLDIR)/ses/usr/sbin/ses

libnet:
	-mkdir -p libnet/lib
	$(MAKE) -C libnet CC=$(CC) AR=$(AR) RANLIB=$(RANLIB)


httpd: nvram shared hsiab matrixssl
	$(MAKE) -C httpd

httpd-clean:
	$(MAKE) -C httpd clean


rc: netconf nvram shared hsiab libnet dnsmasq bird dropbear zlib iproute2 dhcpforwarder pppd rflow iptables 

www www-%:


ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) $* INSTALLDIR=$(INSTALLDIR)/www
else
	$(MAKE) -C www/$(WEB_PAGE) $* INSTALLDIR=$(INSTALLDIR)/www
endif

www-distclean www-clean:
ifeq ($(KROMOGUI),y)
	$(MAKE) -C kromo/$(WEB_PAGE) clean
else
	$(MAKE) -C www/$(WEB_PAGE) clean
endif


busybox-config: 
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config rc net-tools
ifeq ($(CONFIG_DIST),"micro")
#	cp busybox/.config_micro busybox/sysdeps/linux/defconfig
	cp busybox/.config_micro busybox/.config
	cd busybox && make oldconfig
else
ifeq ($(CONFIG_DIST),"mini")
#	cp busybox/.config_mini busybox/sysdeps/linux/defconfig
	cp busybox/.config_mini busybox/.config
	cd busybox && make oldconfig
else
#	cp busybox/.config_std busybox/sysdeps/linux/defconfig
	cp busybox/.config_std busybox/.config
	cd busybox && make oldconfig
endif
endif
	
	make -C busybox clean
	rm -f busybox/busybox
	$(MAKE) -C busybox STRIPTOOL=$(STRIP)

busybox-install:
	$(MAKE) -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox install

busybox-clean: busybox-config
	$(MAKE) -C busybox clean

busybox-distclean: busybox-config
	$(MAKE) -C busybox clean
	$(MAKE) -C busybox/scripts/config clean
	rm -f busybox/.depend busybox/include/config.h busybox/scripts/mkdep

dnsmasq:
	$(MAKE) -C dnsmasq/contrib/wrt CFLAGS="$(COPTS) -DNEED_PRINTF"
	$(MAKE) -C dnsmasq COPTS=-DHAVE_BROKEN_RTC CFLAGS="$(COPTS) -DNEED_PRINTF"

dnsmasq-install:
	install -D dnsmasq/contrib/wrt/lease_update.sh $(INSTALLDIR)/dnsmasq/etc/lease_update.sh
	install -D dnsmasq/contrib/wrt/dhcp_release $(INSTALLDIR)/dnsmasq/usr/sbin/dhcp_release
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq

dnsmasq-clean:
	$(MAKE) -C dnsmasq clean
	$(MAKE) -C dnsmasq/contrib/wrt clean


iptables:
	$(MAKE) -C iptables.linksys BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR)
	$(MAKE) -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR)

iptables-devel:
	$(MAKE) -C iptables install-devel KERNEL_DIR=$(LINUXDIR)


iptables-install:
ifeq ($(CONFIG_IPTABLES),y)
#	install -d $(INSTALLDIR)/iptables/usr/lib/iptables
#	install iptables/extensions/*.so $(INSTALLDIR)/iptables/usr/lib/iptables
#	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/iptables/*.so
	install -D iptables/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables
#	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables
#	install -D iptables/iptables-restore $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
#	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables-restore

        ifeq ($(CONFIG_L7),y)
	  install -d $(TARGETDIR)/etc/l7-protocols
	  cp -rp l7/* $(TARGETDIR)/etc/l7-protocols/
        endif
else
        # So that generic rule does not take precedence
	@true
endif

iptables-clean:
	-$(MAKE) -C iptables.linksys KERNEL_DIR=$(LINUXDIR) clean
	-$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) clean

iptables-distclean:
	-$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) distclean

netconf: iptables


pptpd:
ifeq ($(CONFIG_PPTPD),y)
	$(MAKE) -C pptpd
else
	@true
endif

pptpd-clean:
	$(MAKE) -C pptpd clean

pptpd-install:
ifeq ($(CONFIG_PPTPD),y)
	install -D pptpd/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	install -D pptpd/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	install -D pptpd/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	install -D pptpd/config/pptp.webservices $(INSTALLDIR)/pptpd/etc/config/pptp.webservices
else
        # So that generic rule does not take precedence
	@true
endif

pipsec:
ifeq ($(CONFIG_PIPSEC),y)
	$(MAKE) -C pipsec
else
	@true
endif

pipsec-clean:
	$(MAKE) -C pipsec clean

pipsec-install:
ifeq ($(CONFIG_PIPSEC),y)
	install -D pipsec/pipsecd $(INSTALLDIR)/pipsec/usr/sbin/pipsecd
	$(STRIP) $(INSTALLDIR)/pipsec/usr/sbin/pipsecd
else
        # So that generic rule does not take precedence
	@true
endif

snmp:
ifeq ($(CONFIG_SNMP),y)
	cd snmp && . ./conf.sh
	$(MAKE) -C snmp 
else
	@true
endif

snmp-clean:
	$(MAKE) -C snmp clean
	rm -f $(INSTALLDIR)/etc/snmp

snmp-install:
ifeq ($(CONFIG_SNMP),y)
	install -D snmp/agent/snmpd $(INSTALLDIR)/snmp/usr/sbin/snmpd
	install -D snmp/config/snmp.webservices $(INSTALLDIR)/snmp/etc/config/snmp.webservices
	$(STRIP) $(INSTALLDIR)/snmp/usr/sbin/snmpd
	ln -sf /tmp/etc/snmp $(TARGETDIR)/etc/snmp
else
        # So that generic rule does not take precedence
	@true
endif

wol:
	cd wol && ./configure --disable-nls --host=mipsel-linux CC=$(CC) CFLAGS="$(COPTS)"
	$(MAKE) -C wol

wol-clean:
	$(MAKE) -C wol clean

wol-install:
	install -D wol/src/wol $(INSTALLDIR)/wol/usr/sbin/wol
	$(STRIP) $(INSTALLDIR)/wol/usr/sbin/wol

dhcpforwarder:
	cd dhcpforwarder && ./configure --host=$(ARCH)-linux-elf --sysconfdir=/tmp/dhcp-fwd CC=$(ARCH)-linux-uclibc-gcc CFLAGS="-Os -mips32 -mtune=mips32 -funit-at-a-time -pipe -DHAVE_MALLOC=1 -Drpl_malloc=malloc" 
	$(MAKE) -C dhcpforwarder

dhcpforwarder-install:
	@true
#install -D dhcpforwarder/dhcp-fwd $(INSTALLDIR)/dhcpforwarder/usr/sbin/dhcp-fwd
#$(STRIP) $(INSTALLDIR)/dhcpforwarder/usr/sbin/dhcp-fwd

dhcpforwarder-clean:
	$(MAKE) -C dhcpforwarder clean

rtpproxy:
	$(MAKE) -C rtpproxy

rtpproxy-install:

	install -D rtpproxy/rtpproxy $(INSTALLDIR)/rtpproxy/usr/bin/rtpproxy
	
rtpproxy-clean:
	$(MAKE) -C rtpproxy clean

shat:
	make -C shat/src

shat-install:
	install -D shat/src/shatd $(INSTALLDIR)/shat/usr/sbin/shatd
	install -D shat/src/shatc $(INSTALLDIR)/shat/usr/sbin/shatc
	install -D shat/config/shat.webhotspot $(INSTALLDIR)/shat/etc/config/shat.webhotspot
	install -D shat/config/shat.nvramconfig $(INSTALLDIR)/shat/etc/config/shat.nvramconfig
	install -D shat/config/shat.startup $(INSTALLDIR)/shat/etc/config/shat.startup

	
shat-clean:
	make -C shat/src clean

radauth:
ifeq ($(CONFIG_CHILLILOCAL),y)
	install -D chillispot/config/fon.nvramconfig $(INSTALLDIR)/chillispot/etc/config/fon.nvramconfig
	install -D chillispot/config/fon.webhotspot $(INSTALLDIR)/chillispot/etc/config/fon.webhotspot
endif
	$(MAKE) -C radauth

radauth-install:
	install -D radauth/config/wrt-radauth.startup $(INSTALLDIR)/radauth/etc/config/wrt-radauth.startup
	install -D radauth/wrt-radauth $(INSTALLDIR)/radauth/usr/sbin/wrt-radauth
#	install -D radauth/radius-client $(INSTALLDIR)/radauth/usr/sbin/radius-client
#	install -D radauth/radiusallow $(INSTALLDIR)/radauth/usr/sbin/radiusallow
#	install -D radauth/radiusdisallow $(INSTALLDIR)/radauth/usr/sbin/radiusdisallow
	install -D radauth/macupd $(INSTALLDIR)/radauth/usr/sbin/macupd
ifeq ($(CONFIG_RADLOCAL),y)
	install -D chillispot/config/iradius.nvramconfig $(INSTALLDIR)/chillispot/etc/config/iradius.nvramconfig
	install -D chillispot/config/iradius.webhotspot $(INSTALLDIR)/chillispot/etc/config/iradius.webhotspot
	install -D chillispot/config/iradius.startup $(INSTALLDIR)/chillispot/etc/config/iradius.startup
endif
	
radauth-clean:
	$(MAKE) -C radauth clean
	
samba:
	$(MAKE) -C samba/source bin/smbmnt
	$(MAKE) -C samba/source bin/smbmount

samba-install:
	install -D samba/source/bin/smbmnt $(INSTALLDIR)/samba/usr/sbin/smbmnt
	install -D samba/source/bin/smbmount $(INSTALLDIR)/samba/usr/sbin/smbmount
	install -D samba/smb.conf $(INSTALLDIR)/samba/usr/lib/smb.conf
	install -D samba/config/sambafs.webconfig $(INSTALLDIR)/samba/etc/config/sambafs.webconfig
	install -D samba/config/sambafs.startup $(INSTALLDIR)/samba/etc/config/sambafs.startup
	install -D samba/config/sambafs.nvramconfig $(INSTALLDIR)/samba/etc/config/sambafs.nvramconfig


samba-clean:
	$(MAKE) -C samba/source clean

mmc:
	$(MAKE) -C mmc

mmc-install:
	install -D  mmc/mmc.o $(INSTALLDIR)/mmc/lib/modules/2.4.33-rc2/mmc.o
	install -D  mmc/config/mmc.webconfig $(INSTALLDIR)/mmc/etc/config/mmc.webconfig
	install -D  mmc/config/mmc.startup $(INSTALLDIR)/mmc/etc/config/mmc.startup
	install -D  mmc/config/mmc.nvramconfig $(INSTALLDIR)/mmc/etc/config/mmc.nvramconfig

mmc-clean:
	$(MAKE) -C mmc clean
ser:

	$(MAKE) -C ser modules all ARCH=mipsel CFLAGS="$(COPTS)" extra_defs="-DUSE_PTHREAD_MUTEX "


ser-install:
#	$(MAKE) -C ser install ARCH=mipsel CC=mipsel-linux-uclibc-gcc CFLAGS="-Os -pipe -mips32 -mtune=mips32" BASEDIR=$(INSTALLDIR)
	
	install -D ser/ser $(INSTALLDIR)/ser/usr/sbin/ser
	mkdir -p $(INSTALLDIR)/ser/usr/lib/ser/modules
	mkdir -p $(INSTALLDIR)/ser/etc/config 
	cp -a ser/etc/config/* $(INSTALLDIR)/ser/etc/config/
	cp -a $(SER_MODULE_FILES) $(INSTALLDIR)/ser/usr/lib/ser/modules/
	install -D ser/scripts/sc $(INSTALLDIR)/ser/usr/sbin/serctl
	mkdir -p $(INSTALLDIR)/ser/etc/ser
	install -D ser/etc/ser.cfg $(INSTALLDIR)/ser/etc/ser/ser.cfg
	install -D ser/etc/version $(INSTALLDIR)/ser/etc/ser/version
	install -D ser/etc/subscriber $(INSTALLDIR)/ser/etc/ser/subscriber
	install -D ser/etc/location $(INSTALLDIR)/ser/etc/ser/location
	install -D ser/etc/aliases $(INSTALLDIR)/ser/etc/ser/aliases
	install -D ser/etc/S78sipath $(INSTALLDIR)/ser/etc/init.d/S78sipath
	install -D ser/etc/ip-up $(INSTALLDIR)/ser/etc/ppp/ip-up
	mv $(INSTALLDIR)/ser/usr/lib/ser/modules/nathelper.so $(INSTALLDIR)/ser/usr/lib/ser/modules/inner_nathelper.so




ser-clean:
	$(MAKE) -C ser clean
	rm -rf $(INSTALLDIR)/ser/*
	rm -rf $(INSTALLDIR)/www/*
			
nocat:
	cd glib && ./config.sh
	make -C glib
	cd nocat && . ./config.sh
	make -C nocat

nocat-clean:
	make -C nocat clean
	

nocat-install:
	install -D nocat/src/splashd $(INSTALLDIR)/nocat/usr/sbin/splashd
	$(STRIP) $(INSTALLDIR)/nocat/usr/sbin/splashd
	mkdir -p ${INSTALLDIR}/nocat/etc
	ln -sf /tmp/etc/nocat.conf $(INSTALLDIR)/nocat/etc/nocat.conf
	mkdir -p $(INSTALLDIR)/nocat/usr/libexec
	cp -r nocat/libexec/iptables $(INSTALLDIR)/nocat/usr/libexec/nocat
ifeq ($(CONFIG_RAMSKOV),y)
	install -D nocat/config_redirect/nocat.webhotspot $(INSTALLDIR)/nocat/etc/config/nocat.webhotspot
	install -D nocat/config_redirect/nocat.nvramconfig $(INSTALLDIR)/nocat/etc/config/nocat.nvramconfig
	install -D nocat/config_redirect/nocat.startup $(INSTALLDIR)/nocat/etc/config/nocat.startup
	install -D nocat/config_redirect/nocat.header $(INSTALLDIR)/nocat/etc/config/nocat.header
	install -D nocat/config_redirect/nocat.footer $(INSTALLDIR)/nocat/etc/config/nocat.footer
else
	install -D nocat/config/nocat.webhotspot $(INSTALLDIR)/nocat/etc/config/nocat.webhotspot
	install -D nocat/config/nocat.nvramconfig $(INSTALLDIR)/nocat/etc/config/nocat.nvramconfig
	install -D nocat/config/nocat.startup $(INSTALLDIR)/nocat/etc/config/nocat.startup
endif
http-redirect:
	@true

http-redirect-clean:
	@true

http-redirect-install:
	install -D http-redirect/http-redirect.nvramconfig $(INSTALLDIR)/http-redirect/etc/config/http-redirect.nvramconfig
	install -D http-redirect/http-redirect.webhotspot $(INSTALLDIR)/http-redirect/etc/config/http-redirect.webhotspot
	install -D http-redirect/http-redirect.startup $(INSTALLDIR)/http-redirect/etc/config/http-redirect.startup


smtp-redirect:
	@true

smtp-redirect-clean:
	@true

smtp-redirect-install:
	install -D smtp-redirect/smtp-redirect.nvramconfig $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.nvramconfig
	install -D smtp-redirect/smtp-redirect.webhotspot $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.webhotspot
	install -D smtp-redirect/smtp-redirect.startup $(INSTALLDIR)/smtp-redirect/etc/config/smtp-redirect.startup

		
chillispot:
	cd chillispot && ./configure --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) -DHAVE_MALLOC=1 -Drpl_malloc=malloc"
	$(MAKE) -C chillispot

chillispot-install:
ifneq ($(CONFIG_FON),y)
	install -D chillispot/config/chillispot.nvramconfig $(INSTALLDIR)/chillispot/etc/config/chillispot.nvramconfig
	install -D chillispot/config/chillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispot.webhotspot
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
	install -D chillispot/config/fon.nvramconfig $(INSTALLDIR)/chillispot/etc/config/fon.nvramconfig
	install -D chillispot/config/fon.webhotspot $(INSTALLDIR)/chillispot/etc/config/fon.webhotspot
endif
	install -D chillispot/src/chilli $(INSTALLDIR)/chillispot/usr/sbin/chilli
	$(STRIP) $(INSTALLDIR)/chillispot/usr/sbin/chilli

chillispot-clean:
	$(MAKE) -C chillispot clean

frottle:
ifeq ($(CONFIG_FROTTLE),y)
	$(MAKE) -C frottle 
else
	@true
endif

frottle-clean:
	$(MAKE) -C frottle clean

frottle-install:
ifeq ($(CONFIG_FROTTLE),y)
	install -D frottle/frottle $(INSTALLDIR)/frottle/usr/sbin/frottle
	$(STRIP) $(INSTALLDIR)/frottle/usr/sbin/frottle
else
        # So that generic rule does not take precedence
	@true
endif


e2fsprogs:
	cd e2fsprogs && ./configure --host=mipsel-linux CFLAGS="-Os" CC="$(CROSS_COMPILE)gcc $(COPTS)" --with-gnu-ld --disable-rpath --disable-shared --enable-static --enable-elf-shlibs --enable-dynamic-e2fsck
	make -C e2fsprogs

e2fsprogs-install:
	mkdir -p $(INSTALLDIR)/e2fsprogs/sbin
	cp e2fsprogs/misc/mke2fs.static $(INSTALLDIR)/e2fsprogs/sbin/mke2fs


ebtables:
	$(MAKE) -C ebtables static BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) KERNEL_INCLUDES=$(LINUXDIR)/include

ebtables-install:
ifeq ($(CONFIG_EBTABLES),y)
	install -D ebtables/ebtables $(INSTALLDIR)/ebtables/usr/sbin/ebtables
#	install -d $(INSTALLDIR)/ebtables/usr/lib
#	install ebtables/extensions/*.so $(INSTALLDIR)/ebtables/usr/lib/
#	install ebtables/*.so $(INSTALLDIR)/ebtables/usr/lib/
#	$(STRIP) $(INSTALLDIR)/ebtables/usr/sbin/ebtables
else
        # So that generic rule does not take precedence
	@true
endif

ebtables-clean:
	-$(MAKE) -C ebtables KERNEL_DIR=$(LINUXDIR) clean


ntpclient-install:
	install -D ntpclient/ntpclient $(INSTALLDIR)/ntpclient/usr/sbin/ntpclient
	$(STRIP) $(INSTALLDIR)/ntpclient/usr/sbin/ntpclient

# AhMan March 19 2005
pppoe pppoe-%:
	$(MAKE) -C ppp/pppoecd $* INSTALLDIR=$(INSTALLDIR)/pppoe

ppp:
	$(MAKE) -C rp-pppoe-3.5/src
ppp-clean:
	$(MAKE) -C rp-pppoe-3.5/src clean

ppp-install:
	install -D rp-pppoe-3.5/src/pppoe $(INSTALLDIR)/ppp/usr/sbin/pppoe
#	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe
ifeq ($(CONFIG_PPPOERELAY),y)
	install -D rp-pppoe-3.5/src/pppoe-relay $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
	install -D rp-pppoe-3.5/config/pppoe-relay.nvramconfig $(INSTALLDIR)/ppp/etc/config/pppoe-relay.nvramconfig
	install -D rp-pppoe-3.5/config/pppoe-relay.webservices $(INSTALLDIR)/ppp/etc/config/pppoe-relay.webservices
	install -D rp-pppoe-3.5/config/pppoe-relay.startup $(INSTALLDIR)/ppp/etc/config/pppoe-relay.startup
#	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-relay
endif
	install -D rp-pppoe-3.5/config/rppppoe.startup $(INSTALLDIR)/ppp/etc/config/rppppoe.startup
	install -D rp-pppoe-3.5/config/rppppoe.nvramconfig $(INSTALLDIR)/ppp/etc/config/rppppoe.nvramconfig

ifeq ($(CONFIG_PPPOESERVER),y)
	install -D rp-pppoe-3.5/src/pppoe-server $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-server
endif
ifeq ($(CONFIG_PPPOESNIFF),y)
	install -D rp-pppoe-3.5/src/pppoe-sniff $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
	$(STRIP) $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
else
	rm -f $(INSTALLDIR)/ppp/usr/sbin/pppoe-sniff
endif

udhcpd-clean:
	make -C udhcpc clean
	make -C udhcpd clean
	
udhcpd: shared nvram
	make -C udhcpc
	make -C udhcpd

udhcpd-install:
	install -D udhcpd/udhcpd $(INSTALLDIR)/udhcpd/usr/sbin/udhcpd
	install -D udhcpd/dumpleases $(INSTALLDIR)/udhcpd/usr/sbin/dumpleases
	install -D udhcpd/config/dhcpd.webservices $(INSTALLDIR)/udhcpd/etc/config/dhcpd.webservices
	install -D udhcpd/config/dhcpd.startup $(INSTALLDIR)/udhcpd/etc/config/dhcpd.startup
	$(STRIP) $(INSTALLDIR)/udhcpd/usr/sbin/udhcpd
	$(STRIP) $(INSTALLDIR)/udhcpd/usr/sbin/dumpleases
	install -D udhcpc/udhcpc $(INSTALLDIR)/udhcpd/usr/sbin/udhcpc
#	cd $(INSTALLDIR)/udhcpd/usr/sbin && ln -sf udhcpd udhcpc

upnp: netconf nvram shared

upnp-distclean: upnp-clean
	rm -f $(SRCBASE)/tools/upnp/upnp/linux/libupnp.a

wlconf: nvram shared

vlan:
	$(MAKE) -C vlan CROSS=$(CROSS_COMPILE) STRIPTOOL=$(STRIP)

vlan-install:
	$(MAKE) -C vlan CROSS=$(CROSS_COMPILE) STRIPTOOL=$(STRIP) INSTALLDIR=$(INSTALLDIR) install

vlan-clean:
	$(MAKE) -C vlan clean

vlan-distclean: vlan-clean
	rm -f vlan/vconfig

nas nas-clean nas-distclean:
        # So that generic rule does not take precedence
	@true

nas-install:
ifeq ($(CONFIG_MSSID),y)
	install -D nas/nas $(INSTALLDIR)/nas/usr/sbin/nas
else
	install -D nas/nas.v23 $(INSTALLDIR)/nas/usr/sbin/nas
endif
	$(STRIP) $(INSTALLDIR)/nas/usr/sbin/nas
	cd $(INSTALLDIR)/nas/usr/sbin && ln -sf nas nas4not && ln -sf nas nas4wds

########################################################################
# Below is added by CT

tftpd tftpd-clean tftpd-distclean:
        # So that generic rule does not take precedence
	@true

tftpd-install:
	install -D tftpd/tftpd $(INSTALLDIR)/tftpd/usr/sbin/tftpd
	$(STRIP) $(INSTALLDIR)/tftpd/usr/sbin/tftpd

cron-install:
	#install -d $(INSTALLDIR)/cron/etc/cron.d
	install -D cron/cron $(INSTALLDIR)/cron/usr/sbin/cron
	$(STRIP) $(INSTALLDIR)/cron/usr/sbin/cron

pptp-client:
	$(MAKE) -C pptp-client

pptp-client-clean:
	$(MAKE) -C pptp-client clean

pptp-client-install:
ifneq ($(CONFIG_NEWMEDIA),y)
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
	install -D pptp-client/config/pptpd_client.ip-up $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-up
	install -D pptp-client/config/pptpd_client.ip-down $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-down
	install -D pptp-client/config/pptpd_client.nvramconfig $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.nvramconfig
	install -D pptp-client/config/pptpd_client.webservices $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.webservices
	install -D pptp-client/config/pptpd_client.options $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.options
	install -D pptp-client/config/pptpd_client.sh $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.sh
	install -D pptp-client/config/pptpd_client.startup $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.startup
	install -D pptp-client/config/pptpd_client.vpn $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.vpn
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
else
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
	install -D pptp-client/config2/pptpd_client.ip-up $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-up
	install -D pptp-client/config2/pptpd_client.ip-down $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.ip-down
	install -D pptp-client/config2/pptpd_client.nvramconfig $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.nvramconfig
	install -D pptp-client/config2/pptpd_client.webconfig $(INSTALLDIR)/pptp-client/etc/config/1pptpd_client.webconfig
	install -D pptp-client/config2/pptpd_client.options $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.options
	install -D pptp-client/config2/pptpd_client.sh $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.sh
	install -D pptp-client/config2/pptpd_client.wanup $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.wanup
	install -D pptp-client/config2/pptpd_client.vpn $(INSTALLDIR)/pptp-client/etc/config/pptpd_client.vpn
	install -D pptp-client/pptp $(INSTALLDIR)/pptp-client/usr/sbin/pptp
endif

netstatnat:
	$(MAKE) -C netstatnat

netstatnat-clean:
	$(MAKE) -C netstatnat clean

netstatnat-install:
	install -D netstatnat/netstat-nat $(INSTALLDIR)/netstatnat/usr/sbin/netstat-nat
	$(STRIP) $(INSTALLDIR)/netstatnat/usr/sbin/netstat-nat

pppd-symlinks:
	- (cd pppd.new ; \
	ln -s linux/Makefile.top Makefile ; \
	ln -s Makefile.linux pppd/Makefile ; \
	ln -s Makefile.linux chat/Makefile ; \
	ln -s Makefile.linux pppd/plugins/Makefile ; \
	ln -s Makefile.linux pppd/plugins/radius/Makefile ; \
	ln -s Makefile.linux pppd/plugins/rp-pppoe/Makefile ; \
	ln -s Makefile.linux pppd/plugins/pppoatm/Makefile ; \
	ln -s Makefile.linux pppdump/Makefile ; \
	ln -s Makefile.linux pppstats/Makefile ; \
	)

pppd: pppd-symlinks
	$(MAKE) -C pppd.new/pppd
ifeq ($(CONFIG_PPPSTATS),y)
	$(MAKE) -C pppd.new/pppstats
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	$(MAKE) -C pppd.new/pppd/plugins/radius
endif
#	$(MAKE) -C pppd.new/pppd/plugins/pppoatm


pppd-clean pppd-distclean: pppd-symlinks
	$(MAKE) -C pppd.new/pppd clean

pppd-install:
	install -D pppd.new/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_PPPSTATS),y)
	install -D pppd.new/pppstats/pppstats $(INSTALLDIR)/pppd/usr/sbin/pppstats
else
	rm -rf $(INSTALLDIR)/pppd/usr/sbin/pppstats
endif
ifeq ($(CONFIG_RADIUSPLUGIN),y)
	install -D pppd.new/pppd/plugins/radius/radius.so $(INSTALLDIR)/pppd/usr/lib/pppd/radius.so
	install -D pppd.new/pppd/plugins/radius/radattr.so $(INSTALLDIR)/pppd/usr/lib/pppd/radattr.so
	install -D pppd.new/pppd/plugins/radius/radrealms.so $(INSTALLDIR)/pppd/usr/lib/pppd/radrealms.so
	install -d $(INSTALLDIR)/pppd/etc/radiusclient
	cp pppd.new/pppd/plugins/radius/radiusclient/etc/issue $(INSTALLDIR)/pppd/etc/radiusclient/issue
	cp pppd.new/pppd/plugins/radius/radiusclient/etc/port-id-map $(INSTALLDIR)/pppd/etc/radiusclient/port-id-map
	cp pppd.new/pppd/plugins/radius/radiusclient/etc/dictionary $(INSTALLDIR)/pppd/etc/radiusclient/dictionary
	cp pppd.new/pppd/plugins/radius/radiusclient/etc/dictionary.microsoft $(INSTALLDIR)/pppd/etc/radiusclient/dictionary.microsoft
	chmod 0644 $(INSTALLDIR)/pppd/etc/radiusclient/*
else
	rm -rf $(INSTALLDIR)/pppd/usr/lib/pppd/rad*.so
	rm -rf $(INSTALLDIR)/pppd/etc/radiusclient
endif
	rm -rf $(INSTALLDIR)/pppd/usr/lib/pppd/rp-pppoe.so

#	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/pppd
ifeq ($(CONFIG_PPPSTATS),y)
	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/pppstats
endif

# ipupdate-install:
# #	@true
# 	install -D ipupdate/ez-ipupdate $(INSTALLDIR)/ipupdate/usr/sbin/ez-ipupdate
# 	$(STRIP) $(INSTALLDIR)/ipupdate/usr/sbin/ez-ipupdate

inadyn:
	$(MAKE) -C inadyn

inadyn-install:
	install -D inadyn/bin/linux/inadyn $(INSTALLDIR)/inadyn/usr/sbin/inadyn
	$(STRIP) $(INSTALLDIR)/inadyn/usr/sbin/inadyn

inadyn-clean:
	$(MAKE) -C inadyn clean

zebra:
ifeq ($(CONFIG_ZEBRA),y)
	$(MAKE) -C quagga CC=$(CC)
endif

zebra-install:
ifeq ($(CONFIG_ZEBRA),y)
	install -D quagga/zebra/zebra $(INSTALLDIR)/zebra/usr/sbin/zebra
	install -D quagga/ripd/ripd $(INSTALLDIR)/zebra/usr/sbin/ripd
	install -D quagga/ospfd/ospfd $(INSTALLDIR)/zebra/usr/sbin/ospfd
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/zebra
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ripd
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ospfd
else
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/zebra
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/ripd
	rm -rf $(INSTALLDIR)/zebra/usr/sbin/ospfd
endif

bird-symlinks:
	$(MAKE) -C bird/obj .dir-stamp
	mkdir -p bird/obj/conf bird/obj/lib
	- (cd bird/obj/conf ; \
	ln -s ../../conf/cf-lex.l cf-lex.l ; \
	ln -s ../../conf/conf.c conf.c ; \
	ln -s ../../conf/conf.h conf.h ; \
	ln -s ../../sysdep/unix/config.Y config.Y ; \
	ln -s ../../sysdep/unix/krt.Y krt.Y ; \
	ln -s ../../sysdep/linux/netlink/netlink.Y netlink.Y ; \
	)
	- (cd bird/obj/lib ; \
	ln -s ../../lib/birdlib.h birdlib.h ; \
	ln -s ../../lib/bitops.c bitops.c ; \
	ln -s ../../lib/bitops.h bitops.h ; \
	ln -s ../../lib/checksum.c checksum.c ; \
	ln -s ../../lib/checksum.h checksum.h ; \
	ln -s ../../sysdep/unix/endian.h endian.h ; \
	ln -s ../../lib/event.c event.c ; \
	ln -s ../../lib/event.h event.h ; \
	ln -s ../../sysdep/unix/io.c io.c ; \
	ln -s ../../lib/ip.c ip.c ; \
	ln -s ../../lib/ip.h ip.h ; \
	ln -s ../../lib/ipv4.c ipv4.c ; \
	ln -s ../../lib/ipv4.h ipv4.h ; \
	ln -s ../../sysdep/unix/krt.c krt.c ; \
	ln -s ../../sysdep/unix/krt.h krt.h ; \
	ln -s ../../sysdep/linux/netlink/krt-iface.h krt-iface.h ; \
	ln -s ../../sysdep/linux/netlink/krt-scan.h krt-scan.h ; \
	ln -s ../../sysdep/linux/netlink/krt-set.h krt-set.h ; \
	ln -s ../../lib/lists.c lists.c ; \
	ln -s ../../lib/lists.h lists.h ; \
	ln -s ../../sysdep/unix/log.c log.c ; \
	ln -s ../../sysdep/unix/main.c main.c ; \
	ln -s ../../lib/md5.c md5.c ; \
	ln -s ../../lib/md5.h md5.h ; \
	ln -s ../../lib/mempool.c mempool.c ; \
	ln -s ../../sysdep/linux/netlink/netlink.c netlink.c ; \
	ln -s ../../lib/patmatch.c patmatch.c ; \
	ln -s ../../lib/printf.c printf.c ; \
	ln -s ../../sysdep/unix/random.c random.c ; \
	ln -s ../../lib/resource.c resource.c ; \
	ln -s ../../lib/resource.h resource.h ; \
	ln -s ../../lib/slab.c slab.c ; \
	ln -s ../../lib/slists.c slists.c ; \
	ln -s ../../lib/slists.h slists.h ; \
	ln -s ../../lib/socket.h socket.h ; \
	ln -s ../../lib/string.h string.h ; \
	ln -s ../../sysdep/linux/sysio.h sysio.h ; \
	ln -s ../../sysdep/unix/timer.h timer.h ; \
	ln -s ../../lib/unaligned.h unaligned.h ; \
	ln -s ../../sysdep/unix/unix.h unix.h ; \
	ln -s ../../lib/xmalloc.c xmalloc.c ; \
	)

bird: bird-symlinks
	$(MAKE) -C bird

bird-install:
	@true
#	install -D bird/bird $(INSTALLDIR)/bird/usr/sbin/bird
#	$(STRIP) $(INSTALLDIR)/bird/usr/sbin/bird

bird-clean:
	$(MAKE) -C bird clean

bird-distclean:
	$(MAKE) -C bird clean
	rm -rf bird/obj/nest bird/obj/filter bird/obj/proto
	rm -f bird/obj/.dir-stamp

shorewall-install:
	install -D shorewall/common.def     $(INSTALLDIR)/shorewall/usr/sbin/common.def
	install -D shorewall/firewall       $(INSTALLDIR)/shorewall/usr/sbin/firewall
	install -D shorewall/functions      $(INSTALLDIR)/shorewall/usr/sbin/functions
	install -D shorewall/shorewall      $(INSTALLDIR)/shorewall/usr/sbin/shorewall
	install -D shorewall/shorewall.conf $(INSTALLDIR)/shorewall/usr/sbin/shorewall.conf
	install -D shorewall/version        $(INSTALLDIR)/shorewall/usr/sbin/version

shorewall-clean:
	$(MAKE) -C shorewall clean


bpalogin-install:
	install -D bpalogin/bpalogin $(INSTALLDIR)/bpalogin/usr/sbin/bpalogin
	$(STRIP) $(INSTALLDIR)/bpalogin/usr/sbin/bpalogin

dropbear: zlib
	cd dropbear && ./configure --host=$(ARCH)-linux --disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-libutil CC=$(ARCH)-linux-uclibc-gcc CFLAGS="-I../zlib $(COPTS) -L../zlib" LDFLAGS="-L../zlib" host_alias=$(ARCH)-linux
	$(MAKE) -C dropbear PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" SCPPROGRESS=1 MULTI=1

dropbear-install:
	install -D dropbear/config/sshd.webservices $(INSTALLDIR)/dropbear/etc/config/sshd.webservices
#	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/sbin/dropbear
#	$(STRIP) $(INSTALLDIR)/dropbear/usr/sbin/dropbear
#	cd $(INSTALLDIR)/dropbear/usr/sbin && ln -sf dropbear dropbearconvert && ln -sf dropbear dropbearkey && ln -sf dropbear dbclient
#	mkdir -p $(INSTALLDIR)/dropbear/usr/bin
#	cd $(INSTALLDIR)/dropbear/usr/bin && ln -sf /usr/sbin/dropbear ssh && ln -sf /usr/sbin/dropbear scp

iproute2:
	$(MAKE) -C iproute2 all

iproute2-install:
	chmod 0644 iproute2/tc/tc
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/tc
#	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/tc
	chmod 0644 iproute2/ip/ip
	install -D iproute2/ip/ip $(INSTALLDIR)/iproute2/usr/sbin/ip
#	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/ip
ifeq ($(CONFIG_WSHAPER),y)
	chmod 0644 iproute2/wshaper.htb
	install -D iproute2/wshaper.htb $(INSTALLDIR)/iproute2/usr/sbin/wshaper
else
	@true
endif

ifeq ($(CONFIG_SVQOS),y)
	chmod 0644 iproute2/svqos.htb
	install -D iproute2/svqos.htb $(INSTALLDIR)/iproute2/usr/sbin/svqos
	chmod 0644 iproute2/svqos.hfsc
	install -D iproute2/svqos.hfsc $(INSTALLDIR)/iproute2/usr/sbin/svqos2
else
	@true
endif

adm6996:
ifeq ($(CONFIG_MSSID),y)
	$(MAKE) -C adm6996.v24
else
	$(MAKE) -C adm6996
endif

adm6996-install:
ifeq ($(CONFIG_MSSID),y)
	install -D adm6996.v24/adm6996.o $(INSTALLDIR)/adm6996/lib/modules/2.4.33-rc2/kernel/drivers/net/adm6996/adm6996.o
else
	install -D adm6996/adm6996.o $(INSTALLDIR)/adm6996/lib/modules/2.4.33-rc2/kernel/drivers/net/adm6996/adm6996.o
endif
#	$(STRIP) $(INSTALLDIR)/adm6996/lib/adm6996.o

igmp-proxy-clean:
	make -C igmp-proxy clean

igmp-proxy:
	make -C igmp-proxy

igmp-proxy-install:
	install -D igmp-proxy/igmprt $(INSTALLDIR)/igmp-proxy/usr/sbin/igmprt



parprouted:
ifeq ($(CONFIG_PARPROUTED),y)
	$(MAKE) -C parprouted
else
	@true
endif

parprouted-clean:
	$(MAKE) -C parprouted clean

parprouted-install:
ifeq ($(CONFIG_PARPROUTED),y)
	install -D parprouted/parprouted $(INSTALLDIR)/parprouted/usr/sbin/parprouted
	$(STRIP) $(INSTALLDIR)/parprouted/usr/sbin/parprouted
else
        # So that generic rule does not take precedence
	@true
endif

wpa_supplicant:
ifeq ($(CONFIG_WPA_SUPPLICANT),y)
	$(MAKE) -C wpa_supplicant
else
	@true
endif

wpa_supplicant-clean:
	$(MAKE) -C wpa_supplicant clean

wpa_supplicant-install:
ifeq ($(CONFIG_WPA_SUPPLICANT),y)
	install -D wpa_supplicant/wpa_supplicant $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_supplicant
	install -D wpa_supplicant/wpa_passphrase $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_passphrase
	install -D wpa_supplicant/wpa_cli $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_supplicant
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_passphrase
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
else
	# So that generic rule does not take precedence
	@true
endif

madwifi:
	cd $(HAL)/linux && make TARGET=mips-le-elf clean \
	&&  make TARGET=mips-le-elf \
	&& make TARGET=mips-le-elf release)
	
	make -C madwifi/madwifi KERNELPATH=$(LINUXDIR) TARGET=mips-le-elf ATH_RATE=ath_rate/sample

madwifi-clean:
	make -C madwifi/madwifi clean KERNELPATH=$(LINUXDIR) TARGET=mips-le-elf ATH_RATE=ath_rate/sample

madwifi-install:
	make -C madwifi/madwifi/tools BINDIR=$(INSTALLDIR)/madwifi/bin install

#	mkdir -p $(INSTALLDIR)/madwifi/lib/modules/2.4.33-pre2
#	cp -fpR ./madwifi-ng/net80211/wlan*.o \
#		./madwifi-ng/openhal/ath_hal.o \
#		./madwifi-ng/ath_rate/sample/ath_rate_sample.o \
#		./madwifi-ng/ath/ath_pci.o \
#		$(INSTALLDIR)/madwifi/lib/modules/2.4.33-pre2/
#	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
#	cp -fpR madwifi-ng/tools/athstats \
#		madwifi-ng/tools/athdebug \
#		madwifi-ng/tools/80211stats \
#		madwifi-ng/tools/80211debug \
#		madwifi-ng/tools/athctrl \
#		madwifi-ng/tools/athkey \
#		madwifi-ng/tools/wlanconfig \
#		$(INSTALLDIR)/madwifi/usr/sbin


sputnik:
	@true

sputnik-clean:
	@true

sputnik-install:
	mkdir -p $(INSTALLDIR)/sputnik/etc/config
	cp -fpR sputnik/config/* $(INSTALLDIR)/sputnik/etc/config
	mkdir -p $(INSTALLDIR)/sputnik/www/images
	cp -fpR sputnik/images/* $(INSTALLDIR)/sputnik/www/images
	install -D sputnik/$(ARCH)/sputnik $(INSTALLDIR)/sputnik/usr/sbin/sputnik
	install -D sputnik/$(ARCH)/libiksemel.so $(INSTALLDIR)/sputnik/usr/lib/libiksemel.so

openvpn:
	cd lzo && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS)"
	make -C lzo
	make -C openssl
	rm -f openssl/*.so*
ifeq ($(CONFIG_NEWMEDIA),y)
	cd openvpn && ./configure --host=$(ARCH)-linux CPPFLAGS="-I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --enable-password-save --enable-management --disable-socks --enable-lzo --enable-small --enable-server --disable-http CFLAGS="$(COPTS)" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs"
	make -C openvpn clean
else
	cd openvpn && ./configure --host=$(ARCH)-linux CPPFLAGS="-I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --disable-management --disable-socks --enable-lzo --enable-small --enable-server --disable-http --enable-password-save CFLAGS="$(COPTS)" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs"
	make -C openvpn clean
endif
	make -C openvpn

openvpn-install:
	install -D openvpn/openvpn $(INSTALLDIR)/openvpn/usr/sbin/openvpn
ifneq ($(CONFIG_NEWMEDIA),y)	
	install -D openvpn/config/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config/openvpn.webservices $(INSTALLDIR)/openvpn/etc/config/openvpn.webservices
else
	install -D openvpn/config2/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config2/openvpn.webservices $(INSTALLDIR)/openvpn/etc/config/openvpn.webservices
endif

net-tools:
	make -C net-tools
	make -C net-tools arp

net-tools-install:
	@true
#	install -D net-tools/ifconfig $(INSTALLDIR)/net-tools/sbin/ifconfig
#	install -D net-tools/arp $(INSTALLDIR)/net-tools/sbin/arp

wireless-tools-clean:
	make -C wireless-tools clean

wireless-tools:
	make -C wireless-tools CC=$(CC) CFLAGS="$(COPTS)" all iwmulticall

wireless-tools-install:
	mkdir -p $(INSTALLDIR)/wireless-tools
	make -C wireless-tools PREFIX="$(INSTALLDIR)/wireless-tools" INSTALL_DIR="$(INSTALLDIR)/wireless-tools/usr/sbin" INSTALL_LIB="$(INSTALLDIR)/usr/lib" install install-iwmulticall
	rm -rf $(INSTALLDIR)/wireless-tools/man
	rm -rf $(INSTALLDIR)/wireless-tools/include

olsrd-clean:
	make -C olsrd OS=linux clean

olsrd:
	make -C olsrd OS=linux
	make -C olsrd OS=linux libs

olsrd-install:
	install -D olsrd/olsrd $(INSTALLDIR)/olsrd/usr/sbin/olsrd
	install -D olsrd/lib/dyn_gw/olsrd_dyn_gw.so.0.3 $(INSTALLDIR)/olsrd/usr/lib/olsrd_dyn_gw.so
	install -D olsrd/lib/httpinfo/olsrd_httpinfo.so.0.1 $(INSTALLDIR)/olsrd/usr/lib/olsrd_httpinfo.so


rflow-install:
	install -D rflow/config/rflow.nvramconfig $(INSTALLDIR)/rflow/etc/config/rflow.nvramconfig
	install -D rflow/config/rflow.webservices $(INSTALLDIR)/rflow/etc/config/rflow.webservices
	install -D rflow/config/rflow.startup $(INSTALLDIR)/rflow/etc/config/rflow.startup
	install -D rflow/config/macupd.startup $(INSTALLDIR)/rflow/etc/config/macupd.startup
	install -D rflow/config/macupd.nvramconfig $(INSTALLDIR)/rflow/etc/config/macupd.nvramconfig	

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



