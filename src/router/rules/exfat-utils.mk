exfat-utils-configure:
	cd exfat-utils && ./autogen.sh
	cd exfat-utils && ./configure --host=$(ARCH)-linux --prefix=/usr --libdir=/usr/lib \
	    CC="$(CC) $(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE  -DNEED_PRINTF -std=gnu89" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

exfat-utils:
	make -C exfat-utils

exfat-utils-clean:
	make -C exfat-utils clean

exfat-utils-install:
	make -C exfat-utils install DESTDIR=$(INSTALLDIR)/exfat-utils
	rm -rf $(INSTALLDIR)/exfat-utils/usr/share
	rm -f $(INSTALLDIR)/exfat-utils/usr/sbin/dump.exfat
	rm -f $(INSTALLDIR)/exfat-utils/usr/sbin/exfatlabel
	rm -f $(INSTALLDIR)/exfat-utils/usr/sbin/exfat2img
	rm -f $(INSTALLDIR)/exfat-utils/usr/lib/*.a
	rm -f $(INSTALLDIR)/exfat-utils/usr/lib/*.la

