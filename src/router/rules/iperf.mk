iperf-configure:
	cd iperf && ./bootstrap.sh
	cd iperf && ./configure --host=$(ARCH)-linux --disable-shared --without-openssl --prefix=/usr --libdir=/usr/lib CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"

iperf:
	$(MAKE) -C iperf/src iperf3

iperf-clean:
	if test -e "iperf/Makefile"; then make -C iperf clean; fi
	@true

iperf-install:
	make -C iperf install-exec DESTDIR=$(INSTALLDIR)/iperf
	-mv $(INSTALLDIR)/iperf/usr/lib64/* $(INSTALLDIR)/iperf/usr/lib
	rm -rf $(INSTALLDIR)/iperf/usr/lib

