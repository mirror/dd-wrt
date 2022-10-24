inadyn-configure:
	cd inadynv2/libconfuse && ./autogen.sh
	cd inadynv2/libconfuse && ./configure --prefix=/usr \
					--disable-examples \
					--host=$(ARCH)-linux-elf \
					CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
					AR_FLAGS="cru $(LTOPLUGIN)" \
					RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	$(MAKE) -C inadynv2/libconfuse

	cd inadynv2 && ./autogen.sh
	cd inadynv2 && ./configure --prefix=/usr \
		--enable-reduced \
		--disable-ssl \
		--host=$(ARCH)-linux-elf \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		confuse_CFLAGS="-I$(TOP)/inadynv2/libconfuse/src" \
		confuse_LIBS="-L$(TOP)/inadynv2/libconfuse/src/.libs -lconfuse" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	$(MAKE) -C inadynv2


inadyn:
	$(MAKE) -C inadynv2/libconfuse
	$(MAKE) -C inadynv2

inadyn-install:
	install -D inadyn/bin/linux/inadyn $(INSTALLDIR)/inadyn/usr/sbin/inadyn
	$(STRIP) $(INSTALLDIR)/inadyn/usr/sbin/inadyn

inadyn-clean:
	$(MAKE) -C inadynv2/libconfuse clean
	$(MAKE) -C inadynv2 clean



