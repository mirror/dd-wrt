
httpd: nvram shared hsiab matrixssl wolfssl www wireless-tools
#	$(MAKE) -C httpd/axTLS
#	$(MAKE) www
	$(MAKE) -C httpd

httpd-clean:
	$(MAKE) -C httpd clean
	-rm -f $(TOP)/register/*.o


