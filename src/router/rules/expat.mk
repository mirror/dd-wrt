expat-configure:
	cd expat && ./buildconf.sh
	cd expat && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-shared --enable-static \
		--without-docbook --without-examples --without-tests --without-getrandom \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	
expat:
	$(MAKE) -C expat

expat-install:
	@true

expat-clean:
	-$(MAKE) -C expat clean

.PHONY: expat expat-configure expat-install expat-clean


#Compiler options from OpenWRT
# -DDOCBOOK_TO_MAN=OFF -DEXPAT_BUILD_TOOLS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_DOCS=OFF 
# -DEXPAT_WITH_LIBBSD=OFF -DEXPAT_ENABLE_INSTALL=ON -DEXPAT_DTD=ON -DEXPAT_NS=OFF -DEXPAT_DEV_URANDOM=OFF

