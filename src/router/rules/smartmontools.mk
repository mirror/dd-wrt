smartmontools-configure:
	cd smartmontools && ./autogen.sh
	cd smartmontools && ./configure --host=$(ARCH)-linux-uclibc \
		CPPFLAGS="$(COPTS)  $(MIPS16_OPT) $(LTO) -fPIC -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="$(COPTS)  $(MIPS16_OPT) $(LTO) -fPIC -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="$(COPTS)  $(MIPS16_OPT) $(LTO) -fPIC -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS=" $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		--prefix=/usr --libdir=/usr/lib --with-nvme-devicescan

smartmontools:
	$(MAKE) -C smartmontools svnversion.h
	$(MAKE) -C smartmontools

smartmontools-clean:
	$(MAKE) -C smartmontools clean

smartmontools-install:
	$(MAKE) -C smartmontools install DESTDIR=$(INSTALLDIR)/smartmontools
	rm -rf $(INSTALLDIR)/smartmontools/usr/share
	rm -f $(INSTALLDIR)/smartmontools/usr/lib/*.la
	rm -f $(INSTALLDIR)/smartmontools/usr/lib/*.a
	rm -f $(INSTALLDIR)/smartmontools/usr/sbin/update-smart-drivedb
