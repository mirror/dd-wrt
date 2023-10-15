batman-adv:
ifneq ($(KERNELVERSION),6.1)
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" all
	$(MAKE) -C batctl all CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -lm"
else
	@true
endif

batman-adv-clean:
ifneq ($(KERNELVERSION),6.1)
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" clean
	$(MAKE) -C batctl CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -lm" clean
	@true
else
	@true
endif

batman-adv-install:
ifneq ($(KERNELVERSION),6.1)
	install -D batman-adv/batman-adv.ko $(INSTALLDIR)/batman-adv/lib/batman-adv/batman-adv.ko
	$(ARCH)-linux-strip --strip-unneeded --remove-section=.comment --remove-section=.pdr --remove-section=.mdebug.abi32 $(INSTALLDIR)/batman-adv/lib/batman-adv/*
	install -D batctl/batctl $(INSTALLDIR)/batman-adv/usr/sbin/batctl
	install -D batctl/config/batstart.sh $(INSTALLDIR)/batman-adv/usr/sbin/batstart.sh
else
	@true
endif
