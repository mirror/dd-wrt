libtirpc-configure: zlib
	cd libtirpc && ./bootstrap
	cd libtirpc && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fcommon -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib" --disable-gssapi --disable-static --prefix=/usr

libtirpc: zlib
	make -C libtirpc

libtirpc-clean:
	make -C libtirpc clean

libtirpc-install:
	make -C libtirpc install DESTDIR=$(INSTALLDIR)/libtirpc
	rm -rf $(INSTALLDIR)/libtirpc/etc
	mv $(INSTALLDIR)/libtirpc/usr/etc $(INSTALLDIR)/libtirpc/etc
	rm -rf $(INSTALLDIR)/libtirpc/usr/include
	rm -rf $(INSTALLDIR)/libtirpc/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/libtirpc/usr/share
	rm -f $(INSTALLDIR)/libtirpc/usr/lib/*.a
	rm -f $(INSTALLDIR)/libtirpc/usr/lib/*.la
