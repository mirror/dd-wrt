radvd-configure:
	cd radvd/libdaemon && ./configure --disable-nls --disable-shared --enable-static --disable-lynx --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections "  ac_cv_func_setpgrp_void=yes ; make
	-cd radvd && aclocal && autoconf && automake -a && cd .. 
	cd radvd/flex && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections"; \
	cd .. && ./configure --host=$(ARCH)-linux CHECK_CFLAGS=no CHECK_LIBS=no CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/radvd/flex -ffunction-sections -fdata-sections -Wl,--gc-sections " DAEMON_CFLAGS="-I$(TOP)/radvd/libdaemon" DAEMON_LIBS="-L$(TOP)/radvd/libdaemon/libdaemon/.libs  -ldaemon" LDFLAGS="-L$(TOP)/radvd/flex -L$(TOP)/libutils -lutils -L$(TOP)/nvram -lnvram -Wl,-rpath,$(TOP)/jansson/src/.libs  -ffunction-sections -fdata-sections -Wl,--gc-sections"  --with-flex=$(TOP)/radvd/flex; \
	
radvd-clean:
	if test -e "radvd/Makefile"; then make -C radvd/flex clean; make -C radvd clean; make -C radvd/libdaemon clean; fi
		

radvd:
	make -C radvd/libdaemon
	make -C radvd/flex libfl.a
	make -C radvd

radvd-install:
ifeq ($(CONFIG_IPV6),y)
	install -d $(INSTALLDIR)/radvd/usr/sbin 
	install radvd/radvd $(INSTALLDIR)/radvd/usr/sbin
	install radvd/radvdump $(INSTALLDIR)/radvd/usr/sbin
endif

