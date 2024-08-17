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
SNMP_EXTRACFLAGS+=-DHAVE_MADWIFI -I$(TOP)/madwifi.dev/madwifi.dev -include $(TOP)/madwifi.dev/madwifi.dev/include/compat.h -I$(TOP)/wireless-tools -I$(TOP)/shared -DHEADERS_KERNEL -Wno-incompatible-pointer-types
SNMP_EXTRAMIB=,ddwrt/ddwrt
ifeq ($(CONFIG_WIRELESS_TOOLS),y)
SNMP_EXTRALIB=-liw -lnl-tiny
else
SNMP_EXTRALIB=-lnl-tiny
endif
SNMP_EXTRACFLAGS+=-DHAVE_ATH9K

SNMP_MIB_MODULES_INCLUDED = \
	agent/extend \
	agentx \
	host/hr_device \
	host/hr_disk \
	host/hr_filesys \
	host/hr_network \
	host/hr_partition \
	host/hr_proc \
	host/hr_storage \
	host/hr_system \
	ieee802dot11 \
	if-mib/ifXTable \
	ip-mib/inetNetToMediaTable \
	mibII/at \
	mibII/icmp \
	mibII/ifTable \
	mibII/ip \
	mibII/snmp_mib \
	mibII/sysORTable \
	mibII/system_mib \
	mibII/tcp \
	mibII/udp \
	mibII/vacm_context \
	mibII/vacm_vars \
	tunnel \
	ucd-snmp/extensible \
	ucd-snmp/loadave \
	ucd-snmp/memory \
	ucd-snmp/pass \
	ucd-snmp/pass_persist \
	ucd-snmp/proc \
	ucd-snmp/vmstat \
	util_funcs \
	utilities/execute \

SNMP_MIB_MODULES_EXCLUDED = \
	agent_mibs \
	disman/event \
	disman/schedule \
	hardware \
	host \
	if-mib \
	ip-mib \
	mibII \
	notification \
	notification-log-mib \
	snmpv3mibs \
	target \
	tcp-mib \
	ucd_snmp \
	udp-mib \
	utilities \
	snmpv3/snmpEngine \
	snmpv3/snmpMPDStats \
	snmpv3/usmConf \
	snmpv3/usmStats \
	snmpv3/usmUser \
	ucd-snmp/disk \
	ucd-snmp/dlmod \

SNMP_TRANSPORTS_INCLUDED = Callback UDP Unix

SNMP_TRANSPORTS_EXCLUDED = TCP TCPIPv6


snmp-configure: nvram libutils
	cd snmp && rm -f config.cache
	cd snmp && libtoolize
	cd snmp && aclocal
	cd snmp && autoconf
	cd snmp && autoheader
	cd snmp && autoreconf -vfi
	-cd snmp && mkdir build_mac80211
	-cd snmp && mkdir build_standard
	-cd snmp && cd build_mac80211 && ../configure  --quiet \
				--prefix=/usr \
				--libdir=/usr/lib \
				--target=$(ARCH)-linux \
				--host=$(ARCH) \
				--with-cc="$(CC)" \
				--with-ar=$(ARCH)-linux-uclibc-ar \
				--with-endianness=$(SNMP_ENDIAN) \
				--with-cflags="$(COPTS) $(MIPS16_OPT) $(SNMP_EXTRACFLAGS) $(LTO) -DHAVE_LINUX_RTNETLINK_H -I$(SSLPATH)/include -D_GNU_SOURCE -DCAN_USE_SYSCTL=1 -I$(TOP)/libnl-tiny/include -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/shared -I$(TOP)/../include.v24" \
				--with-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(SSLPATH) -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram $(SNMP_EXTRALIB)" \
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
				--sysconfdir=/tmp \
				--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
				--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED) ddwrt/ddwrt" \
				--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
				--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
				--disable-ipv6 \
				--with-defaults \
				--without-efence \
				--without-rsaref \
				--without-kmem-usage \
				--without-rpm \
				--without-openssl \
				--without-dmalloc \
				--without-zlib \
				--disable-perl-cc-checks \
				--disable-embedded-perl \
				--without-perl-modules \
				--with-opaque-special-types \
				AR_FLAGS="cru $(LTOPLUGIN)" \
				RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd snmp && cd build_standard && ../configure  --quiet \
				--prefix=/usr \
				--libdir=/usr/lib \
				--target=$(ARCH)-linux \
				--host=$(ARCH) \
				--with-cc="$(CC)" \
				--with-ar=$(ARCH)-linux-uclibc-ar \
				--with-endianness=$(SNMP_ENDIAN) \
				--with-cflags="$(COPTS) $(MIPS16_OPT) $(LTO) -DHAVE_LINUX_RTNETLINK_H -I$(SSLPATH)/include -D_GNU_SOURCE -DCAN_USE_SYSCTL=1 -I$(TOP)/libnl-tiny/include -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/shared -I$(TOP)/../include.v24" \
				--with-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(SSLPATH)" \
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
				--sysconfdir=/tmp \
				--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
				--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED)" \
				--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
				--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
				--disable-ipv6 \
				--with-defaults \
				--without-efence \
				--without-rsaref \
				--without-kmem-usage \
				--without-rpm \
				--without-openssl \
				--without-dmalloc \
				--without-zlib \
				--disable-perl-cc-checks \
				--disable-embedded-perl \
				--without-perl-modules \
				--with-opaque-special-types \
				AR_FLAGS="cru $(LTOPLUGIN)" \
				RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

snmp:
	install -D snmp/config/snmp.webservices httpd/ej_temp/snmp.webservices
ifeq ($(CONFIG_SNMP),y)
ifeq ($(CONFIG_ATH9K),y)
	$(MAKE) -C snmp/build_mac80211 LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram $(SNMP_EXTRALIB)"
else
	$(MAKE) -C snmp/build_standard LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram"
endif
else
	@true
endif

snmp-clean:
	-$(MAKE) -C snmp/build_mac80211/ clean
	-$(MAKE) -C snmp/build_standard/ clean
	rm -f $(INSTALLDIR)/etc/snmp

snmp-install:
ifeq ($(CONFIG_SNMP),y)

ifeq ($(CONFIG_ATH9K),y)
	$(MAKE) -C snmp/build_mac80211 LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram $(SNMP_EXTRALIB)" install DESTDIR=$(INSTALLDIR)/snmp
else
	$(MAKE) -C snmp/build_standard LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(SSLPATH) -L$(TOP)/libutils -L$(TOP)/nvram -L$(TOP)/libnl-tiny -L$(TOP)/wireless-tools -lshutils -lutils -lwireless -lnvram" install DESTDIR=$(INSTALLDIR)/snmp
endif
	rm -rf $(INSTALLDIR)/snmp/etc
	rm -rf $(INSTALLDIR)/snmp/usr/include
	rm -rf $(INSTALLDIR)/snmp/usr/lib
	rm -rf $(INSTALLDIR)/snmp/usr/share
		
	install -D snmp/config/snmp.webservices $(INSTALLDIR)/snmp/etc/config/snmp.webservices
	$(STRIP) $(INSTALLDIR)/snmp/usr/sbin/snmpd
	ln -sf /tmp/etc/snmp $(INSTALLDIR)/snmp/etc/snmp
ifneq ($(CONFIG_SNMP-UTILS),y)
	rm -rf $(INSTALLDIR)/snmp/usr/bin
	rm -f $(INSTALLDIR)/snmp/usr/sbin/snmptrapd
endif
else
        # So that generic rule does not take precedence
	@true
endif
