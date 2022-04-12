expat-configure:
	cd expat && ./buildconf.sh
	cd expat && ./configure --prefix=/usr --host=$(ARCH)-linux --enable-shared --disable-static --libdir=/usr/lib \
		--without-docbook --without-examples --without-tests --without-getrandom \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	
expat:
	$(MAKE) -C expat

expat-install:
	$(MAKE) -C expat install DESTDIR=$(INSTALLDIR)/expat
	rm -rf $(INSTALLDIR)/expat/usr/include
	rm -rf $(INSTALLDIR)/expat/usr/bin
	rm -rf $(INSTALLDIR)/expat/usr/share
	rm -rf $(INSTALLDIR)/expat/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/expat/usr/lib/cmake
	rm -f $(INSTALLDIR)/expat/usr/lib/*.la
	rm -f $(INSTALLDIR)/expat/usr/lib/*.a

expat-clean:
	-$(MAKE) -C expat clean

.PHONY: expat expat-configure expat-install expat-clean


#Compiler options from OpenWRT
# -DDOCBOOK_TO_MAN=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_DOCS=OFF 
# -DEXPAT_WITH_LIBBSD=OFF -DEXPAT_ENABLE_INSTALL=ON -DEXPAT_DTD=ON -DEXPAT_NS=OFF -DEXPAT_DEV_URANDOM=OFF

