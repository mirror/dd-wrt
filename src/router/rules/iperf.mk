iperf-configure:
	cd iperf && libtoolize
	cd iperf && aclocal
	cd iperf && autoconf
	cd iperf && autoheader
	cd iperf && automake --add-missing
	cd iperf && ./configure --host=$(ARCH)-linux --disable-shared --without-openssl --prefix=/usr --libdir=/usr/lib \
		CC="$(CC)" \
		CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		CPPFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
		LDFLAGS="$(LDLTO) -latomic -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

iperf:
	$(MAKE) -C iperf/src iperf3

iperf-clean:
	if test -e "iperf/Makefile"; then make -C iperf clean; fi
	@true

iperf-install:
	make -C iperf install-exec DESTDIR=$(INSTALLDIR)/iperf
	-mv $(INSTALLDIR)/iperf/usr/lib64/* $(INSTALLDIR)/iperf/usr/lib
	rm -rf $(INSTALLDIR)/iperf/usr/lib

