zlib-configure:
	cd zlib && CFLAGS="$(COPTS) $(MIPS16_OPT)" && ./configure

zlib-clean:
	make -C zlib clean

zlib:
	make -C zlib
	rm -f zlib/libz.a

zlib-install:
	install -D zlib/libz.so.1.2.10 $(INSTALLDIR)/zlib/usr/lib/libz.so.1.2.10
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.10 libz.so.1  ; true
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.10 libz.so  ; true

