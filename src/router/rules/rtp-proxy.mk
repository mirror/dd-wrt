rtpproxy-configure:
	cd rtpproxy && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -Drpl_malloc=malloc $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"

rtpproxy:
	$(MAKE) -C rtpproxy

rtpproxy-install:
	install -D rtpproxy/src/rtpproxy $(INSTALLDIR)/rtpproxy/usr/bin/rtpproxy
	
rtpproxy-clean:
	$(MAKE) -C rtpproxy clean