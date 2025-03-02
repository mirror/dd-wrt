# SMCRoute makefile for installing SMCRoute by egc
# https://github.com/troglobit/smcroute


smcroute-configure:
	cd smcroute && ./autogen.sh
	cd smcroute && ./configure --prefix=/usr --sysconfdir=/etc \
	    --without-libcap --build=$(BUILD) --host=$(ARCH)-linux-uclibc  \ 
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-static $(LDLTO)  -ffunction-sections -fdata-sections -Wl,--gc-sections" AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	


smcroute:
	install -D smcroute/config/smcrouted.webservices httpd/ej_temp/
	$(MAKE) -C smcroute CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF"

smcroute-install:
	#$(MAKE) -C smcroute install-strip DESTDIR=$(INSTALLDIR)/smcroute
	#-rm -rf $(INSTALLDIR)/smcroute/usr/share
	#-rm -rf $(INSTALLDIR)/smcroute/lib
	mkdir -p $(INSTALLDIR)/smcroute/etc/config/
	install -D smcroute/src/smcrouted $(INSTALLDIR)/smcroute/usr/sbin/smcrouted
	install -D smcroute/src/smcroutectl $(INSTALLDIR)/smcroute/usr/bin/smcroutectl
	install -D smcroute/smcroute.conf $(INSTALLDIR)/smcroute/etc/smcroute.conf
	install -D smcroute/smcroute $(INSTALLDIR)/smcroute/usr/bin/smcroute
	#ln -sf /tmp/smcroute.conf $(INSTALLDIR)/smcroute/etc/smcroute.conf
	install -D smcroute/config/smcrouted.webservices $(INSTALLDIR)/smcroute/etc/config/
	install -D smcroute/config/smcrouted.nvramconfig $(INSTALLDIR)/smcroute/etc/config/	

smcroute-clean:
	$(MAKE) -C smcroute clean

smcroute-distclean:
	$(MAKE) -C smcroute distclean

.PHONY: smcroute-configure smcroute smcroute-install smcroute-clean smcroute-distclean
