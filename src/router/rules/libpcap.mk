libpcap:
#	$(MAKE) -C libpcap CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	$(MAKE) -C libpcap_noring CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
else
	$(MAKE) -C libpcap_noring CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.a 
endif

libpcap-install:
	@true
ifeq ($(CONFIG_LIBPCAP_SHARED),y)
	install -d $(INSTALLDIR)/libpcap/usr/lib
	install libpcap_noring/libpcap.so.1.0.0 $(INSTALLDIR)/libpcap/usr/lib
endif

libpcap-clean:
	$(MAKE) -C libpcap_noring clean

