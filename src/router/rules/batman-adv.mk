batman-adv:
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" all
	$(MAKE) -C batctl all CC="$(CC)" CFLAGS="$(COPTS) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -lm"


batman-adv-clean:
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" clean
	$(MAKE) -C batctl CC="$(CC)" CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -lm" clean
	@true

batman-adv-install:
	install -D batman-adv/batman-adv.ko $(INSTALLDIR)/batman-adv/lib/batman-adv/batman-adv.ko
	strip --strip-unneeded --remove-section=.comment --remove-section=.pdr --remove-section=.mdebug.abi32 $(INSTALLDIR)/batman-adv/lib/batman-adv/*
	install -D batctl/batctl $(INSTALLDIR)/batman-adv/usr/sbin/batctl
	install -D batctl/config/batstart.sh $(INSTALLDIR)/batman-adv/usr/sbin/batstart.sh
