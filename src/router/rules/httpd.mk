httpd: nvram shared hsiab matrixssl
#	$(MAKE) -C httpd/axTLS
	$(MAKE) www
	$(MAKE) -C httpd

httpd-clean:
	$(MAKE) -C httpd clean

