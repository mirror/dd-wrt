httpd: nvram shared hsiab matrixssl
	$(MAKE) www
	$(MAKE) -C httpd

httpd-clean:
	$(MAKE) -C httpd clean

