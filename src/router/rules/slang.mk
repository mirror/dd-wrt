slang-configure:
	cd slang && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS)  $(MIPS16_OPT) -I$(TOP)/zlib -L$(TOP)/zlib -L$(INSTALLDIR)/util-linux/usr/lib" --enable-shared \
		--disable-static \
		--without-png \
		--libdir=/usr/lib \
		--prefix=/usr \
		--enable-debug=no 
	make -C slang clean
	make -C slang

slang:
	make -C slang

slang-clean:
	if test -e "slang/Makefile"; then $(MAKE) -C slang clean; fi

slang-install:
	make -C slang install DESTDIR=$(INSTALLDIR)/slang
	rm -rf $(INSTALLDIR)/slang/usr/bin
	rm -rf $(INSTALLDIR)/slang/usr/etc
	rm -rf $(INSTALLDIR)/slang/usr/include
	rm -rf $(INSTALLDIR)/slang/usr/share
	rm -rf $(INSTALLDIR)/slang/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/slang/usr/lib/slang
