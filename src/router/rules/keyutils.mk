keyutils:
	make -C keyutils

keyutils-clean:
	make -C keyutils clean

keyutils-install:
	make -C keyutils install DESTDIR=$(INSTALLDIR)/keyutils
