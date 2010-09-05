libnltiny:
	$(MAKE) CFLAGS="$(COPTS)" -C libnl-tiny 

libnltiny-clean:
	$(MAKE) -C libnl-tiny clean

libnltiny-install:
	@true
	#install -D libnl-tiny/libnl-tiny.so $(INSTALLDIR)/libnltiny/usr/lib/libnl-tiny.so
