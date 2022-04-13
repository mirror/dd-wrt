libdaemon-configure:
	cd libdaemon && autoreconf -fsi
	cd libdaemon && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-dependency-tracking --disable-shared --enable-static --disable-examples ac_cv_func_setpgrp_void=yes \
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

libdaemon:
	$(MAKE) -C libdaemon

libdaemon-install:
	 @true

libdaemon-clean:
	-$(MAKE) -C libdaemon clean

.PHONY: libdaemon-configure libdaemon libdaemon-install libdaemon-clean
