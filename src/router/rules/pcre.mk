pcre-configure:
	cd pcre && ./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF" --prefix=/usr --disable-xmldoc --enable-utf8 --enable-unicode-properties --disable-pcretest-libreadline --libdir=$(TOP)/pcre/.libs
	touch pcre/*    


pcre:
	$(MAKE) -C pcre CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF"

pcre-clean:
	$(MAKE) -C pcre clean CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF"

pcre-install:
	install -D pcre/.libs/libpcre.so.1 $(INSTALLDIR)/pcre/usr/lib/libpcre.so.1
	install -D pcre/.libs/libpcrecpp.so.0 $(INSTALLDIR)/pcre/usr/lib/libpcrecpp.so.0
	install -D pcre/.libs/libpcreposix.so.0 $(INSTALLDIR)/pcre/usr/lib/libpcreposix.so.0
