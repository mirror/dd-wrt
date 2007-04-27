eou: libnet libpcap
	[ ! -d eou ] || $(MAKE) -C eou

eou-install:
	[ ! -d eou ] || install -D eou/eou $(INSTALLDIR)/eou/usr/sbin/eou
	$(STRIP) $(INSTALLDIR)/eou/usr/sbin/eou