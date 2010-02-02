ap-serv-checkout:
	rm -rf $(TOP)/ap-serv
	svn co svn://svn.dd-wrt.com/private/ap_serv $(TOP)/ap-serv

ap-serv-update:
	svn update $(TOP)/ap-serv


ap-serv: 
	if test -e "ap-serv/Makefile"; then make -C ap-serv; fi
	@true
ap-serv-clean:
	if test -e "ap-serv/Makefile"; then make -C ap-serv clean; fi
	@true

ap-serv-install:
	install -D ap-serv/src/ap_serv $(INSTALLDIR)/ap-serv/usr/sbin/ap_serv
	install -D ap-serv/config/apserv.webservices $(INSTALLDIR)/ap-serv/etc/config/apserv.webservices
	install -D ap-serv/config/apserv.nvramconfig $(INSTALLDIR)/ap-serv/etc/config/apserv.nvramconfig
