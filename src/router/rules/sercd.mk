sercd-configure:
	cd sercd && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DNEED_PRINTF" --prefix=/usr

sercd:
	make -C sercd

sercd-clean:
	make -C sercd clean

sercd-install:
	make -C sercd install DESTDIR=$(INSTALLDIR)/sercd
	rm -rf $(INSTALLDIR)/sercd/usr/man
