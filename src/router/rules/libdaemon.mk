libdaemon-configure:
	cd libdaemon && autoreconf -fsi
	CFLAGS="$(LTO) $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections"
	cd libdaemon && ./configure --prefix=/usr --host=$(ARCH)-linux --disable-dependency-tracking --disable-examples ac_cv_func_setpgrp_void=yes

libdaemon:
	$(MAKE) -C libdaemon
	#$(MAKE) -C libdaemon DESTDIR=$(TOP)/libdaemon/staged install

libdaemon-install:
	install -D libdaemon/libdaemon/.libs/libdaemon.so.0.5.0 $(INSTALLDIR)/libdaemon/usr/lib/libdaemon.so.0.5.0
	cd $(INSTALLDIR)/libdaemon/usr/lib && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so.0

libdaemon-clean:
	-$(MAKE) -C libdaemon clean

.PHONY: libdaemon-configure libdaemon libdaemon-install libdaemon-clean
