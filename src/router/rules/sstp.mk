sstp-client-configure:
	cd sstp-client && ./configure --prefix=/usr CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -I$(TOP)/pppd -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/openssl/include -I$(TOP)/pppd/include -DNEED_PRINTF" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/pppd -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/openssl/include -I$(TOP)/pppd/include -L$(TOP)/libevent/.libs -L$(TOP)/openssl -DNEED_PRINTF" CPPFLAGS="$(COPTS) -I$(TOP)/pppd  -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/pppd/include" --prefix=/usr --host=$(ARCH)-linux
	@true

sstp-client:
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -D_GNU_SOURCE -I$(TOP)/pppd -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/openssl/include -I$(TOP)/pppd/include -DNEED_PRINTF" \
	LDCFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/pppd -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/openssl/include -I$(TOP)/pppd/include -L$(TOP)/openssl -L$(TOP)/libevent/.libs -DNEED_PRINTF" \
	CPPFLAGS="$(COPTS) -I$(TOP)/pppd -I$(TOP)/pppd/include" \
	$(MAKE) -C sstp-client

sstp-client-clean:
	$(MAKE)  -C sstp-client clean

sstp-client-install:
	$(MAKE)  -C sstp-client install DESTDIR=$(INSTALLDIR)/sstp-client
	rm -rf $(INSTALLDIR)/sstp-client/usr/share
	rm -rf $(INSTALLDIR)/sstp-client/usr/include
	rm -rf $(INSTALLDIR)/sstp-client/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/sstp-client/usr/lib/*.a
	rm -f $(INSTALLDIR)/sstp-client/usr/lib/*.la
	mv $(INSTALLDIR)/sstp-client/usr/lib/pppd/2.4.5/*.so $(INSTALLDIR)/sstp-client/usr/lib
	rm -rf $(INSTALLDIR)/sstp-client/usr/lib/pppd