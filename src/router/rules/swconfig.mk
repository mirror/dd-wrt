swconfig-configure:
	@true

swconfig: libnltiny
	make -C swconfig clean CFLAGS="$(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"
	make -C swconfig CFLAGS="$(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

swconfig-clean:
	make -C swconfig clean CFLAGS="$(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/libnl-tiny/include -DCONFIG_LIBNL20 -D_GNU_SOURCE  -DNEED_PRINTF" LDFLAGS="-L$(TOP)/libnl-tiny/ -lnl-tiny" LIBS="-lm -lnl-tiny"

swconfig-install:
	install -D swconfig/swconfig $(INSTALLDIR)/swconfig/usr/sbin/swconfig
	install -D swconfig/libswitch.so $(INSTALLDIR)/swconfig/usr/lib/libswitch.so
