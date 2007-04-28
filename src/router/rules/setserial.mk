setserial-clean:
	$(MAKE) -C setserial-2.17 clean

setserial:
	$(MAKE) -C setserial-2.17 CC=$(CC)  

setserial-install:
	install -D setserial-2.17/setserial $(INSTALLDIR)/setserial/usr/sbin/setserial
	$(STRIP) $(INSTALLDIR)/setserial/usr/sbin/setserial
