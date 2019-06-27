smartmontools-configure:
	cd smartmontools && ./autogen.sh
	cd smartmontools && ./configure --host=$(ARCH)-linux-uclibc \
		CPPFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		--prefix=/usr

smartmontools:
	$(MAKE) -C smartmontools

smartmontools-clean:
	$(MAKE) -C smartmontools clean

smartmontools-install:
	$(MAKE) -C smartmontools install DESTDIR=$(INSTALLDIR)/smartmontools
	rm -rf $(INSTALLDIR)/smartmontools/usr/share
