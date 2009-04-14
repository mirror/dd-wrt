asterisk-configure:
	cd asterisk && ./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS) -DNEED_PRINTF" -without-curl \
	--without-curses \
	--without-gsm \
	--without-imap \
	--without-isdnnet \
	--without-kde \
	--without-misdn \
	--without-nbs \
	--with-ncurses="$(TOP)/ncurses" \
	--without-netsnmp \
	--without-newt \
	--without-odbc \
	--without-ogg \
	--without-osptk \
	--without-oss \
	--without-pri \
	--without-qt \
	--without-radius \
	--without-spandsp \
	--without-suppserv \
	--without-tds \
	--without-termcap \
	--without-tinfo \
	--without-tonezone \
	--without-vorbis \
	--without-vpb \
	--without-zaptel \

#	--with-popt="$(STAGING_DIR)/usr" \

asterisk:
	make -j 4 -C asterisk

asterisk-install:
	make -C asterisk install DESTDIR=$(INSTALLDIR)/asterisk
	make -C asterisk adsi DESTDIR=$(INSTALLDIR)/asterisk
#	make -C ncurses install.libs DESTDIR=$(INSTALLDIR)/ncurses
#	rm -rf $(INSTALLDIR)/ncurses/usr/include
#	rm -f $(INSTALLDIR)/ncurses/usr/lib/*.a

