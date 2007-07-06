bmon:
	cd bmon && ./configure --host=$(ARCH)-linux --disable-rrd --disable-asound --disable-dbi --disable-curses
	make -C bmon

bmon-clean:
	make -C bmon/src clean


