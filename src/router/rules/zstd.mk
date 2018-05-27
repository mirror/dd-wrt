zstd-configure:
	@true

zstd:
	$(MAKE) -C zstd

zstd-clean:
	$(MAKE) -C zstd clean

zstd-install:
	$(MAKE) -C zstd install DESTDIR=$(INSTALLDIR)/zstd LIBDIR=/usr/lib BINDIR=/usr/bin INCLUDEDIR=/usr/include PKGCONFIGDIR=/usr/lib/pkgconfig MAN1DIR=/usr/share/man
	rm -rf $(INSTALLDIR)/zstd/usr/include
	rm -rf $(INSTALLDIR)/zstd/usr/bin
	rm -rf $(INSTALLDIR)/zstd/usr/share
	rm -rf $(INSTALLDIR)/zstd/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/zstd/usr/lib/*.a
	rm -f $(INSTALLDIR)/zstd/usr/lib/*.la