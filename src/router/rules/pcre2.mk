pcre2-configure:
	cd pcre2 && ./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF" --prefix=/usr --disable-xmldoc --enable-utf8 --enable-unicode-properties --disable-pcretest-libreadline --libdir=$(TOP)/pcre2/.libs
	touch $(TOP)/pcre2/*   


pcre2:
	$(MAKE) -C pcre2 CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" CXXFLAGS="$(COPTS) $(MIPS16_OPT)" CPPFLAGS="$(COPTS) $(MIPS16_OPT)"

pcre2-clean:
	$(MAKE) -C pcre2 clean CFLAGS="$(COPTS) $(MIPS16_OPT)"

pcre2-install:
	install -D pcre2/.libs/libpcre2-8.so.0 $(INSTALLDIR)/pcre/usr/lib/libpcre2-8.so.0
	install -D pcre2/.libs/libpcre2-posix.so.3 $(INSTALLDIR)/pcre/usr/lib/libpcre2-posix.so.3
