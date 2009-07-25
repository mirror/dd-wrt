httpd-checkout:
	rm -rf $(TOP)/register
	svn co svn://svn.dd-wrt.com/private/register $(TOP)/register

httpd-update:
	svn update $(TOP)/register


httpd: nvram shared hsiab matrixssl www wireless-tools
#	$(MAKE) -C httpd/axTLS
#	$(MAKE) www
	$(MAKE) -j 4 -C httpd

httpd-clean:
	$(MAKE) -C httpd clean

