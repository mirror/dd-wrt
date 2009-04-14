asterisk-configure:
	cd asterisk && ./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/zlib -L$(TOP)" -without-curl \
	--without-curses \
	--without-gtk \
	--without-gtk2 \
	--without-gsm \
	--without-isdnnet \
	--without-kde \
	--without-misdn \
	--without-nbs \
	--without-x11 \
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
	--without-sdl \
	--without-spandsp \
	--without-suppserv \
	--without-tds \
	--without-termcap \
	--without-tinfo \
	--without-tonezone \
	--without-vorbis \
	--without-vpb \
	--without-zaptel \
	--with-z="$(TOP)/zlib" 

#	--with-popt="$(STAGING_DIR)/usr" \

asterisk:
	make -j 4 -C asterisk

asterisk-install:
	make -C asterisk install DESTDIR=$(INSTALLDIR)/asterisk
	make -C asterisk adsi DESTDIR=$(INSTALLDIR)/asterisk
	rm -rf $(INSTALLDIR)/asterisk/var
	rm -rf $(INSTALLDIR)/asterisk/usr/share
	rm -rf $(INSTALLDIR)/asterisk/usr/include
#	make -C ncurses install.libs DESTDIR=$(INSTALLDIR)/ncurses
#	rm -rf $(INSTALLDIR)/ncurses/usr/include
#	rm -f $(INSTALLDIR)/ncurses/usr/lib/*.a

