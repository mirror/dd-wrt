iftop-configure:
	cd iftop && ./configure --host=$(ARCH)-linux --with-libpcap="$(TOP)/libpcap" CFLAGS="$(COPTS) -I$(TOP)/libpcap -I$(TOP)/ncurses/include" LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/libpcap" CPPFLAGS="$(COPTS) -I$(TOP)/libpcap -I$(TOP)/ncurses/include"

iftop: ncurses
	cd iftop && ./configure --host=$(ARCH)-linux --with-libpcap="$(TOP)/libpcap" CFLAGS="$(COPTS) -I$(TOP)/libpcap -I$(TOP)/ncurses/include" LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/libpcap" CPPFLAGS="$(COPTS) -I$(TOP)/libpcap -I$(TOP)/ncurses/include"
	make -C iftop

iftop-install:
	install -D iftop/iftop $(INSTALLDIR)/iftop/usr/sbin/iftop

