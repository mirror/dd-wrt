ddrescue: 
	$(MAKE) -C ddrescue

ddrescue-install:
	$(MAKE) -C ddrescue install DESTDIR=$(INSTALLDIR)/ddrescue
	rm -rf $(INSTALLDIR)/ddrescue/usr/share

ddrescue-configure:
	cd ddrescue && ./configure CXX=$(ARCH)-linux-g++ --enable-linux --prefix=/usr CXXFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"

ddrescue-clean:
	$(MAKE) -C ddrescue clean

