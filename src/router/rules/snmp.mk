snmp:
ifeq ($(CONFIG_SNMP),y)
	cd snmp && rm -f config.cache
	
	cd snmp && ./configure  --quiet --prefix=/tmp/snmp --host=$(ARCH) --with-cc=$(ARCH)-linux-uclibc-gcc --with-ar=$(ARCH)-linux-uclibc-ar --with-endianness=big --with-cflags="$(COPTS) -DCAN_USE_SYSCTL=1" --enable-mini-agent --disable-debugging --disable-privacy --without-opaque-special-types --with-persistent-directory=/tmp/snmp-persist --with-default-snmp-version=3 --with-sys-contact=root --with-sys-location=Unknown --with-logfile=/dev/null --with-out-transports=UDPIPv6,TCPIPv6,AAL5PVC,IPX,TCP,Unix --enable-shared=no --enable-static --with-gnu-ld --enable-internal-md5 --with-copy-persistent-files=no --without-openssl -sysconfdir=/tmp --with-mib-modules=mibII,host,mibII/ip,mibII/tcp,mibII/udp,mibII/icmp,mibII/var_route,mibII/kernel_linux,qos,ucd_snmp --with-out-mib-modules=agent_mips,agentx,notification,utilities,target --disable-ipv6 --with-defaults --without-efence --without-rsaref --without-kmem-usage --without-rpm --without-dmalloc
#	cd snmp && ./configure  --quiet --prefix=/tmp/snmp --host="$(ARCH)-linux" --with-cc=$(ARCH)-linux-gcc --with-ar=$(ARCH)-linux-ar --with-endianness=big --with-cflags="$(COPTS) -DNETSNMP_CAT_USE_SYSCTL=1" --enable-mini-agent --disable-privacy --without-opaque-special-types --with-persistent-directory=/tmp/snmp-persist --with-default-snmp-version=3 --with-sys-contact=root --with-sys-location=Unknown --with-logfile=/dev/null --with-out-transports=UDPIPv6,TCPIPv6,AAL5PVC,IPX,TCP,Unix --enable-shared=no --enable-static --with-gnu-ld --enable-internal-md5 --with-copy-persistent-files=no --without-openssl -sysconfdir=/tmp --with-mib-modules=mibII,host,mibII/ip,mibII/tcp,mibII/udp,mibII/icmp,mibII/var_route,mibII/kernel_linux,qos,ucd_snmp --with-out-mib-modules=agent_mips,agentx,notification,utilities,target --disable-ipv6 --with-defaults --without-efence --without-rsaref --without-kmem-usage --without-rpm --without-dmalloc
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
	ln -sf /tmp/etc/snmp $(INSTALLDIR)/snmp/etc/snmp
else
        # So that generic rule does not take precedence
	@true
endif