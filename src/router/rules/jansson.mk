jansson-configure:
	cd jansson && autoreconf --force --install
	cd jansson && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" LDFLAGS="-lm"

jansson:
	$(MAKE) -C jansson

jansson-clean:
	if test -e "jansson/Makefile"; then make -C jansson clean; fi
	@true

jansson-install:
	$(MAKE) -C jansson install DESTDIR=$(INSTALLDIR)/jansson
	-rm -rf $(INSTALLDIR)/jansson/usr/include
	-rm -rf $(INSTALLDIR)/jansson/usr/lib/pkgconfig
	-rm -f $(INSTALLDIR)/jansson/usr/lib/*.a
	-rm -f $(INSTALLDIR)/jansson/usr/lib/*.la
