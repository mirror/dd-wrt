expat-configure:
	cd expat && ./buildconf.sh
	mkdir -p expat/static
	mkdir -p expat/dynamic
	-$(MAKE) -C expat distclean
	cd expat/static && ../configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static --libdir=/usr/lib \
		--without-docbook --without-examples --without-tests --without-getrandom \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	cd expat/dynamic && ../configure --prefix=/usr --host=$(ARCH)-linux --enable-shared --disable-static --libdir=/usr/lib \
		--without-docbook --without-examples --without-tests --without-getrandom \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	
expat:
	$(MAKE) -C expat/dynamic
	$(MAKE) -C expat/static

expat-install:
ifeq ($(CONFIG_MDNS_UTILS),y)
	$(MAKE) -C expat/dynamic install DESTDIR=$(INSTALLDIR)/expat
	rm -rf $(INSTALLDIR)/expat/usr/include
	rm -rf $(INSTALLDIR)/expat/usr/bin
	rm -rf $(INSTALLDIR)/expat/usr/share
	rm -rf $(INSTALLDIR)/expat/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/expat/usr/lib/cmake
	rm -f $(INSTALLDIR)/expat/usr/lib/*.la
	rm -f $(INSTALLDIR)/expat/usr/lib/*.a
else
	@true
endif

expat-clean:
	-$(MAKE) -C expat/dynamic clean
	-$(MAKE) -C expat/static clean

.PHONY: expat expat-configure expat-install expat-clean


#Compiler options from OpenWRT
# -DDOCBOOK_TO_MAN=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_DOCS=OFF 
# -DEXPAT_WITH_LIBBSD=OFF -DEXPAT_ENABLE_INSTALL=ON -DEXPAT_DTD=ON -DEXPAT_NS=OFF -DEXPAT_DEV_URANDOM=OFF

