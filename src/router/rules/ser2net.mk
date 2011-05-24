ser2net-configure:
	cd ser2net && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" --prefix=/usr

ser2net:
	make -C ser2net

ser2net-clean:
	if test -e "ser2net/Makefile"; then make -C ser2net clean; fi

ser2net-install:
	make -C ser2net install DESTDIR=$(INSTALLDIR)/ser2net
	rm -rf $(INSTALLDIR)/ser2net/usr/man
