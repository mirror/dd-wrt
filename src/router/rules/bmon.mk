bmon:
	cd bmon/libnl && ./configure --host $(ARCH)-linux --prefix=/usr
	make -C bmon/libnl
	cd bmon && ./configure --host=$(ARCH)-linux --disable-rrd --disable-asound --disable-dbi --disable-curses DEPFLAGS="$(COPTS) -I$(TOP)/bmon/libnl/include"  CFLAGS="$(COPTS) -I$(TOP)/bmon/libnl/include" LDFLAGS="-L$(TOP)/bmon/libnl/lib"
	make -C bmon

bmon-clean:
	make -C bmon/src clean
	make -C bmon/libnl clean


