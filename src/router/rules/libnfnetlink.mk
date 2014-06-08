libnfnetlink-configure:
	cd libnfnetlink && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--enable-shared \
		--disable-static \
		--libdir=$(TOP)/libnfnetlink/src/.libs
		
		#CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


libnfnetlink:
	$(MAKE) -C libnfnetlink CFLAGS="$(COPTS)  $(MIPS16_OPT) -D_GNU_SOURCE -DNEED_PRINTF"

libnfnetlink-install:
	install -D libnfnetlink/src/.libs/libnfnetlink.so.0 $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0
