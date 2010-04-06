aoss-checkout:
	rm -rf $(TOP)/aoss
	svn co svn://svn.dd-wrt.com/private/aoss $(TOP)/aoss

aoss-update:
	svn update $(TOP)/aoss


aoss: 
	if test -e "aoss/Makefile"; then make -C aoss; fi
	@true
aoss-clean:
	if test -e "aoss/Makefile"; then make -C aoss clean; fi
	@true

aoss-install:
	install -D aoss/src/aoss $(INSTALLDIR)/aoss/usr/sbin/aoss
