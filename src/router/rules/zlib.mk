zlib-configure:
	cd zlib && ./configure

zlib-clean:
	make -C zlib clean

zlib:
	make -j 4 -C zlib

zlib-install:
	install -D zlib/libz.so.1.2.8 $(INSTALLDIR)/zlib/usr/lib/libz.so.1.2.8
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.8 libz.so.1  ; true
	cd $(INSTALLDIR)/zlib/usr/lib ; ln -s libz.so.1.2.8 libz.so  ; true

