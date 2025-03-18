JEMALLOC_FLAGS:=--with-lg-hugepage=21 --with-lg-page=12

ifeq ($(ARCH),aarch64)
JEMALLOC_FLAGS:=--with-lg-hugepage=21 --with-lg-page=16
endif
ifeq ($(ARCH),powerpc)
JEMALLOC_FLAGS:=--with-lg-hugepage=21 --with-lg-page=16
endif

jemalloc-configure:
	cd jemalloc && ./autogen.sh
	cd jemalloc && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux --disable-debug --disable-stats --disable-fill --disable-cxx --enable-xmalloc $(JEMALLOC_FLAGS) CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" 

jemalloc:
	$(MAKE) -C jemalloc

jemalloc-clean:
	$(MAKE) -C jemalloc clean

jemalloc-install:
	$(MAKE) -C jemalloc install DESTDIR=$(INSTALLDIR)/jemalloc
	rm -rf $(INSTALLDIR)/jemalloc/usr/bin
	rm -rf $(INSTALLDIR)/jemalloc/usr/include
	rm -rf $(INSTALLDIR)/jemalloc/usr/share
	rm -rf $(INSTALLDIR)/jemalloc/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/jemalloc/usr/lib/*.a
