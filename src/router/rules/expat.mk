expat-configure:
	#cd expat && autoreconf -fsi
	cd expat && ./buildconf.sh
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"
	cd expat && ./configure --prefix=/usr --host=$(ARCH)-linux \
		--without-docbook --without-examples --without-tests --without-getrandom
	
expat: expat-configure
	$(MAKE) -C expat

expat-install:
	install -D expat/lib/.libs/libexpat.so.1.8.8 $(INSTALLDIR)/expat/usr/lib/libexpat.so.1.8.8
	cd $(INSTALLDIR)/expat/usr/lib && ln -sf libexpat.so.1.8.8 libexpat.so.1

expat-clean:
	-$(MAKE) -C expat clean
	rm -f expat/stamp-h1

.PHONY: expat expat-configure expat-install expat-clean


#Compiler options from OpenWRT
# -DDOCBOOK_TO_MAN=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_DOCS=OFF 
# -DEXPAT_WITH_LIBBSD=OFF -DEXPAT_ENABLE_INSTALL=ON -DEXPAT_DTD=ON -DEXPAT_NS=OFF -DEXPAT_DEV_URANDOM=OFF

