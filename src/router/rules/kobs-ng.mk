kobs-ng-configure:
	cd kobs-ng && ./configure --host=$(ARCH)-linux CFLAGS=" $(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -DNEED_PRINTF" --prefix=/usr

kobs-ng:
	make   -C kobs-ng

kobs-ng-clean:
	make   -C kobs-ng clean

kobs-ng-install:
	make   -C kobs-ng install DESTDIR=$(INSTALLDIR)/kobs-ng
	rm -rf $(INSTALLDIR)/kobs-ng/usr/man
	rm -rf $(INSTALLDIR)/kobs-ng/usr/lib

