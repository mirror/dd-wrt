batman-adv:
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" all
	$(MAKE) -C batctl all CC=$(CC) CFLAGS="$(COPTS)"

batman-adv-clean:
	$(MAKE) -C batman-adv KERNELPATH="$(LINUXDIR)" clean
	$(MAKE) -C batctl CC=$(CC) CFLAGS="$(COPTS)" clean
	@true

batman-adv-install:
	install -D batman-adv/batman-adv.ko $(INSTALLDIR)/batman-adv/lib/batman-adv/batman-adv.ko
	strip --strip-unneeded --remove-section=.comment --remove-section=.pdr --remove-section=.mdebug.abi32 $(INSTALLDIR)/batman-adv/lib/batman-adv/*
	install -D batctl/batctl $(INSTALLDIR)/batman-adv/usr/sbin/batctl
