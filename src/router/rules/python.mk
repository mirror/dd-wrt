python-configure:
	cd python && ./configure --host=$(ARCH)-linux --build=$(ARCH) --sysconfdir=/etc \
		--enable-shared \
		--enable-static \
		--prefix=/usr \
		--without-cxx-main \
		--with-threads \
		--with-system-ffi="$(INSTALLDIR)/python/usr" \
		--without-ensurepip \
		--enable-ipv6 \
		CONFIG_SITE="$(TOP)/python/site/config.site" \
		OPT="$(COPTS) -I$(TOP)/zlib" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/python"

python-clean:
	make -C python clean

python:
	make -C python python Parser/pgen
	make -C python sharedmods

python-install:
	make -C python install DESTDIR=$(INSTALLDIR)/python
	rm -rf $(INSTALLDIR)/python/usr/include
	rm -rf $(INSTALLDIR)/python/usr/share
	rm -f $(INSTALLDIR)/python/usr/lib/python3.4/config-3.4m/*.a

