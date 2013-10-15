htpdate-configure:
	@true

htpdate:
	$(MAKE) -C htpdate

htpdate-clean:
	$(MAKE) -C htpdate clean

htpdate-install:
	install -D htpdate/htpdate $(INSTALLDIR)/htpdate/usr/sbin/htpdate
	$(STRIP) $(INSTALLDIR)/htpdate/usr/sbin/htpdate
