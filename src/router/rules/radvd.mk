radvd-configure:
	cd radvd/flex && ./configure --disable-nls --prefix=/usr --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS)"
	cd radvd && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -I$(TOP)/radvd/flex" LDFLAGS="-L$(TOP)/radvd/flex"  --with-flex=$(TOP)/radvd/flex

radvd-clean:
	make -C radvd/flex clean
	make -C radvd clean

radvd:
	make -C radvd/flex libfl.a
	make -C radvd

radvd-install:
	make -C radvd install

