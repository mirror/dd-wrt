
iftop: ncurses
	cd iftop && ./configure --host=$(ARCH)-linux --with-libpcap="$(TOP)/libpcap_noring" CFLAGS="$(COPTS) -I$(TOP)/libpcap_noring -I$(TOP)/ncurses/include" LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/libpcap_noring" CPPFLAGS="$(COPTS) -I$(TOP)/libpcap_noring -I$(TOP)/ncurses/include"
	make -C iftop

iftop-install:
	install -D iftop/iftop $(INSTALLDIR)/iftop/usr/sbin/iftop

