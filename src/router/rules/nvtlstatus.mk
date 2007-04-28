nvtlstatus-clean:
	$(MAKE) -C nvtlstatus CC=$(CC) clean

nvtlstatus:
	$(MAKE) -C nvtlstatus CC=$(CC)  
	$(STRIP) $(INSTALLDIR)/nvtlstatus/usr/sbin/nvtlstatus

nvtlstatus-install:
	install -D nvtlstatus/nvtlstatus $(INSTALLDIR)/nvtlstatus/usr/sbin/nvtlstatus

