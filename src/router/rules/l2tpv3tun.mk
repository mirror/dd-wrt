l2tpv3tun-configure:
	@true

l2tpv3tun:
	$(MAKE) -C l2tpv3tun NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

l2tpv3tun-clean:
	$(MAKE) -C l2tpv3tun clean NL2FOUND=Y NL1FOUND= NLLIBNAME="libnl-tiny" CFLAGS="$(COPTS) -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

l2tpv3tun-install:
	install -D l2tpv3tun/l2tpv3tun $(INSTALLDIR)/l2tpv3tun/usr/sbin/l2tpv3tun
