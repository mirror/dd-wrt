radvd-configure:
	cd radvd/libdaemon && ./configure --disable-nls --disable-shared --enable-static --disable-lynx --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS)"  ac_cv_func_setpgrp_void=yes ; make
	cd radvd/flex && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS)"; \
	cd .. && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/radvd/flex" DAEMON_CFLAGS="-I$(TOP)/radvd/libdaemon" DAEMON_LIBS="-L$(TOP)/radvd/libdaemon/libdaemon/.libs  -ldaemon" LDFLAGS="-L$(TOP)/radvd/flex"  --with-flex=$(TOP)/radvd/flex; \
	
radvd-clean:
	if test -e "radvd/Makefile"; then make -C radvd/flex clean; make -C radvd clean; make -C radvd/libdaemon clean; fi
		

radvd:
	make -C radvd/libdaemon
	make -C radvd/flex libfl.a
	make -C radvd

radvd-install:
	install -d $(INSTALLDIR)/radvd/usr/sbin 
	install radvd/radvd $(INSTALLDIR)/radvd/usr/sbin
	install radvd/radvdump $(INSTALLDIR)/radvd/usr/sbin

