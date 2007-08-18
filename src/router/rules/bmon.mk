bmon-configure:
	cd bmon/libnl && ./configure --host $(ARCH)-linux --prefix=/usr
	cd bmon && ./configure --host=$(ARCH)-linux --disable-rrd --disable-asound --disable-dbi --disable-curses DEPFLAGS="$(COPTS) -I$(TOP)/bmon/libnl/include"  CFLAGS="$(COPTS) -I$(TOP)/bmon/libnl/include" LDFLAGS="-L$(TOP)/bmon/libnl/lib"

bmon:
	make -C bmon/libnl
	make -C bmon

bmon-clean:
	make -C bmon/src clean
	make -C bmon/libnl clean


