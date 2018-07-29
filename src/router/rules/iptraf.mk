iptraf-configure:
	cd iptraf && autoconf
	cd iptraf && ./configure --prefix=/usr --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -I$(TOP)/ncurses/include -L$(TOP)/ncurses/lib" CC="ccache $(ARCH)-linux-uclibc-gcc"

iptraf:
	make -C iptraf

iptraf-clean:
	make -C iptraf clean

iptraf-install:
	make -C iptraf install DESTDIR=$(INSTALLDIR)/iptraf

