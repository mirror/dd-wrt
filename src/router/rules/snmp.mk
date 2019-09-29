ifeq ($(ARCH),arm)
SNMP_ENDIAN=little
endif
ifeq ($(ARCH),aarch64)
SNMP_ENDIAN=little
endif
ifeq ($(ARCH),armeb)
SNMP_ENDIAN=big
endif
ifeq ($(ARCH),i386)
SNMP_ENDIAN=little
endif
ifeq ($(ARCH),x86_64)
SNMP_ENDIAN=little
endif
ifeq ($(ARCH),mips)
SNMP_ENDIAN=big
endif
ifeq ($(ARCH),mips64)
SNMP_ENDIAN=big
endif
ifeq ($(ARCH),mipsel)
SNMP_ENDIAN=little
endif
ifeq ($(ARCH),powerpc)
SNMP_ENDIAN=big
endif
ifeq ($(CONFIG_MADWIFI),y)
SNMP_EXTRACFLAGS+=-DHAVE_MADWIFI -I$(TOP)/madwifi.dev/madwifi.dev -include $(TOP)/madwifi.dev/madwifi.dev/include/compat.h -I$(TOP)/wireless-tools -I$(TOP)/shared -DHEADERS_KERNEL 
SNMP_EXTRAMIB=,ddwrt/ddwrt
ifeq ($(CONFIG_WIRELESS_TOOLS),y)
SNMP_EXTRALIB=-liw -lnl-tiny
else
SNMP_EXTRALIB=-lnl-tiny
endif
endif
ifeq ($(CONFIG_ATH9K),y)
SNMP_EXTRACFLAGS+=-DHAVE_ATH9K
endif
snmp-configure: nvram libutils
	cd snmp && rm -f config.cache
	cd snmp && ./configure  --quiet \
				--prefix=/tmp/snmp \
				--target=$(ARCH)-linux \
				--host=$(ARCH) \
				--with-cc="$(CC)" \
				--with-ar=$(ARCH)-linux-uclibc-ar \
				--with-endianness=$(SNMP_ENDIAN) \
				--with-cflags="$(COPTS) $(MIPS16_OPT) $(SNMP_EXTRACFLAGS) -I$(TOP)/openssl/include -D_GNU_SOURCE -DCAN_USE_SYSCTL=1 -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/shared -I$(TOP)/../include.v24" \
				--with-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram $(SNMP_EXTRALIB)" \
				--enable-mini-agent \
				--disable-debugging \
				--enable-privacy \
				--without-opaque-special-types \
				--with-persistent-directory=/tmp/snmp-persist \
				--with-default-snmp-version=3 \
				--with-sys-contact=root \
				--with-sys-location=Unknown \
				--with-logfile=/dev/null \
				--with-out-transports=UDPIPv6,TCPIPv6,AAL5PVC,IPX,TCP,Unix \
				--enable-shared=no \
				--enable-static \
				--with-gnu-ld \
				--enable-internal-md5 \
				--with-copy-persistent-files=no \
				--with-openssl=$(TOP)/openssl \
				--sysconfdir=/tmp \
				--with-mib-modules=mibII,host,mibII/ip,mibII/tcp,mibII/udp,mibII/icmp,mibII/var_route,mibII/kernel_linux,ucd_snmp$(SNMP_EXTRAMIB) \
				--with-out-mib-modules=host/hr_swrun,agent_mips,agentx,notification,utilities,target,etherlike-mib,notification-log-mib,snmp-notification-mib,tsm-mib,tlstm-lib \
				--disable-ipv6 \
				--with-defaults \
				--without-efence \
				--without-rsaref \
				--without-kmem-usage \
				--without-rpm \
				--without-dmalloc \
				--with-opaque-special-types

snmp:
ifeq ($(CONFIG_SNMP),y)
	$(MAKE) -C snmp LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram $(SNMP_EXTRALIB)"
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
	ln -sf /tmp/etc/snmp $(INSTALLDIR)/snmp/etc/snmp
ifeq ($(CONFIG_SNMP-UTILS),y)
	mkdir -p $(INSTALLDIR)/snmp/usr/bin
	install -D snmp/apps/snmp{get,set,status,test,trap,walk} $(INSTALLDIR)/snmp/usr/bin/
endif
else
        # So that generic rule does not take precedence
	@true
endif
