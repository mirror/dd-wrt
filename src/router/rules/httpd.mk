ifeq ($(CONFIG_SAMBA3),y)
	JANSSON=jansson
endif

httpd: nvram shared hsiab matrixssl www wireless-tools $(JANSSON)
#	$(MAKE) -C httpd/axTLS
#	$(MAKE) www
	$(MAKE) -j 4 -C httpd

httpd-clean:
	$(MAKE) -C httpd clean
	-rm -f $(TOP)/register/*.o


