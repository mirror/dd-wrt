iperf-configure:
	cd iperf && ./configure --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" CPPFLAGS="$(COPTS) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"

iperf:
	$(MAKE) -C iperf

iperf-clean:
	if test -e "iperf/Makefile"; then make -C iperf clean; fi
	@true

iperf-install:
	install -D iperf/src/iperf $(INSTALLDIR)/iperf/usr/sbin/iperf

