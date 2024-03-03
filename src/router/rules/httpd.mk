
httpd: nvram shared hsiab matrixssl wolfssl www wireless-tools
	make -C httpd

httpd-clean:
	make -C httpd clean
	-rm -f $(TOP)/register/*.o


