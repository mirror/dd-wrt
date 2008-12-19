libpcap:
	$(MAKE) -C libpcap CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.so 
	$(MAKE) -C libpcap_noring CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) libpcap.a 

libpcap-install:
	install -d $(INSTALLDIR)/libpcap/usr/lib
	install libpcap/libpcap.so $(INSTALLDIR)/libpcap/usr/lib
	$(STRIP) $(INSTALLDIR)/libpcap/usr/lib/libpcap.so
