gpsd-configure: ncurses
	cd gpsd && ./configure --host=$(ARCH)-linux --disable-fv18 --disable-sirf --disable-python --disable-tsip --disable-garmin --disable-ntrip --disable-evermore --disable-rtcm104 --disable-tripmate --disable-earthmate --disable-shared --without-x CFLAGS="$(COPTS) -DNEED_PRINTF -I../ncurses/include -L../ncurses/lib " --prefix=/usr --enable-ntpshm --enable-pps

gpsd:
	make  -C gpsd

gpsd-clean:
	make  -C gpsd clean

gpsd-install:
	make  -C gpsd install DESTDIR=$(INSTALLDIR)/gpsd
	rm -rf $(INSTALLDIR)/gpsd/usr/man
	rm -rf $(INSTALLDIR)/gpsd/usr/lib
	rm -rf $(INSTALLDIR)/gpsd/usr/share
	rm -rf $(INSTALLDIR)/gpsd/usr/lib64
	rm -rf $(INSTALLDIR)/gpsd/usr/include

