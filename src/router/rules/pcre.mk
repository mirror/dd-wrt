pcre-configure:
	cd pcre && ./configure --host=$(ARCH)-linux-uclibc --disable-xmldoc --enable-utf8 --enable-unicode-properties --disable-pcretest-libreadline

pcre:
	$(MAKE) -C pcre CFLAGS="$(COPTS) -DNEED_PRINTF"

pcre-clean:
	$(MAKE) -C pcre clean CFLAGS="$(COPTS) -DNEED_PRINTF"

pcre-install:
	install -D l2tpv3tun/l2tpv3tun $(INSTALLDIR)/l2tpv3tun/usr/sbin/l2tpv3tun
