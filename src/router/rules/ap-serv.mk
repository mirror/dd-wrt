ap-serv: 
	if test -e "ap-serv/src/Makefile"; then make -C ap-serv/src clean; fi
	if test -e "ap-serv/src/Makefile"; then make -C ap-serv/src; fi
	@true
ap-serv-clean:
	if test -e "ap-serv/src/Makefile"; then make -C ap-serv/src clean; fi
	@true

ap-serv-install:
	install -D ap-serv/ap_serv-$(ARCH)-$(ARCHITECTURE) $(INSTALLDIR)/ap-serv/usr/sbin/ap_serv
	install -D ap-serv/config/apserv.webservices $(INSTALLDIR)/ap-serv/etc/config/apserv.webservices
	install -D ap-serv/config/apserv.nvramconfig $(INSTALLDIR)/ap-serv/etc/config/apserv.nvramconfig
