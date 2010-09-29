aoss: 
	if test -e "aoss/Makefile"; then make -C aoss; fi
	@true
aoss-clean:
	if test -e "aoss/Makefile"; then make -C aoss clean; fi
	@true

aoss-install:
	install -D aoss/src/aoss $(INSTALLDIR)/aoss/usr/sbin/aoss
