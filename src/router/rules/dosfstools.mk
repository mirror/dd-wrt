dosfstools-configure:
	cd dosfstools && libtoolize
	cd dosfstools && aclocal
	cd dosfstools && autoconf
	cd dosfstools && automake --add-missing
	cd dosfstools && ./configure --host=$(ARCH)-linux --prefix=/usr --enable-compat-symlinks --without-udev \
	    CC="$(CC) $(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE  -DNEED_PRINTF -std=gnu89" \
	    LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections"

dosfstools:
	make -C dosfstools

dosfstools-clean:
	make -C dosfstools clean

dosfstools-install:
	make -C dosfstools install DESTDIR=$(INSTALLDIR)/dosfstools
	rm -rf $(INSTALLDIR)/dosfstools/usr/share
	rm -f $(INSTALLDIR)/dosfstools/usr/sbin/dumpexfat
	rm -f $(INSTALLDIR)/dosfstools/usr/sbin/fatlabel
	rm -f $(INSTALLDIR)/dosfstools/usr/sbin/dosfslabel

