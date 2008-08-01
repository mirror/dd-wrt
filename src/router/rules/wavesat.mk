wavesat-configure:
	@true
#	cd wavesat && make OS_TYPE=WAVESAT_TIMESYS_2_6 STATION_TYPE=SUB_STATION LIB_C=uClibc  ARCH_TYPE=TARGET_ARM	dep

wavesat:
	@true	
#	cd wavesat && make OS_TYPE=WAVESAT_TIMESYS_2_6 STATION_TYPE=SUB_STATION LIB_C=uClibc  ARCH_TYPE=TARGET_ARM	all

wavesat-install:
	mkdir -p $(INSTALLDIR)/wavesat/etc/config
	mkdir -p $(INSTALLDIR)/wavesat/sub/
	mkdir -p $(INSTALLDIR)/wavesat/sub/common
	mkdir -p $(INSTALLDIR)/wavesat/sub/lm_scripts
	mkdir -p $(INSTALLDIR)/wavesat/sub/start
	mkdir -p $(INSTALLDIR)/wavesat/sub/utils
	mkdir -p $(INSTALLDIR)/wavesat/lib/modules/$(KERNELRELEASE)/kernel/drivers/net/wimax
	mkdir -p wavesat/install
	cp -f wavesat/config/* $(INSTALLDIR)/wavesat/etc/config
	cp -rf wavesat/sub/arm/ts_2_6/uclibc/* wavesat/install
	mv -f wavesat/install/*.ko $(INSTALLDIR)/wavesat/lib/modules/$(KERNELRELEASE)/kernel/drivers/net/wimax
	cp -rf wavesat/install/* $(INSTALLDIR)/wavesat/sub
	rm -rf wavesat/install
	rm -f $(INSTALLDIR)/wavesat/sub/*.o
	mkdir -p $(TARGETDIR)/lib
	cp -rf $(INSTALLDIR)/wavesat/lib $(TARGETDIR)

wavesat-clean:
	@true
