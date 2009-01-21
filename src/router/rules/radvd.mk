radvd-configure:
	if test ! -e "radvd/Makefile"; then \
	    cd radvd/flex && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS)"; \
	    cd .. && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -I$(TOP)/radvd/flex" LDFLAGS="-L$(TOP)/radvd/flex"  --with-flex=$(TOP)/radvd/flex; \
	fi
	
radvd-clean:
	if test -e "radvd/Makefile"; then make -C make -C radvd/flex clean; make -C radvd clean; fi
		

radvd: radvd-configure
	make -C radvd/flex libfl.a
	make -C radvd

radvd-install:
	install -d $(INSTALLDIR)/radvd/usr/sbin 
	install radvd/radvd $(INSTALLDIR)/radvd/usr/sbin
	install radvd/radvdump $(INSTALLDIR)/radvd/usr/sbin

