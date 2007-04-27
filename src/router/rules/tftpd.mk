tftpd tftpd-clean tftpd-distclean:
        # So that generic rule does not take precedence
	@true

tftpd-install:
	install -D tftpd/tftpd $(INSTALLDIR)/tftpd/usr/sbin/tftpd
	$(STRIP) $(INSTALLDIR)/tftpd/usr/sbin/tftpd

