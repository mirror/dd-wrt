iptraf-configure:
	cd iptraf && ./configure --prefix=/usr --host=$(ARCH)-linux CFLAGS="$(COPTS) -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib"

iptraf:
	make -C iptraf

iptraf-clean:
	make -C iptraf clean

iptraf-install:
	make -C iptraf install DESTDIR=$(INSTALLDIR)/iptraf

