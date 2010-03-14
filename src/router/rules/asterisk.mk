

asterisk-configure:
	cd asterisk && ./configure --host=$(ARCH)-linux-uclibc \
	--without-curl \
	--without-curses \
	--with-gsm=internal \
	--without-gtk \
	--without-gtk2 \
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
	--without-pri \
	--without-qt \
	--without-radius \
	--without-sdl \
	--without-spandsp \
	--without-suppserv \
	--without-tds \
	--without-termcap \
	--without-tinfo \
	--without-vorbis \
	--without-vpb \
	--with-z="$(TOP)/zlib" \
	--disable-xmldoc \
	--without-dahdi \
	--without-gnutls \
	--without-iksemel

asterisk:
	-rm asterisk/menuselect.makeopts
	$(MAKE) -C asterisk \
		include/asterisk/version.h \
		include/asterisk/buildopts.h defaults.h \
		makeopts.embed_rules
	ASTCFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	ASTLDFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	$(MAKE) -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		all
	make -C asterisk

asterisk-install:
	ASTCFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	ASTLDFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	$(MAKE) -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		DESTDIR=$(INSTALLDIR)/asterisk \
		install
	ASTCFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	ASTLDFLAGS="$(COPTS) -DLOW_MEMORY -fPIC" \
	$(MAKE) -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		DESTDIR=$(INSTALLDIR)/asterisk \
		adsi
#	sed 's|/var/lib/asterisk|/usr/lib/asterisk|g' $(INSTALLDIR)/asterisk/etc/asterisk/musiconhold.conf
	rm -rf $(INSTALLDIR)/asterisk/usr/include
	rm -rf $(INSTALLDIR)/asterisk/usr/share
	rm -rf $(INSTALLDIR)/asterisk/usr/lib/static-http
	rm -rf $(INSTALLDIR)/asterisk/usr/lib/sounds
	rm -rf $(INSTALLDIR)/asterisk/usr/lib/images
	rm -rf $(INSTALLDIR)/asterisk/usr/lib/documentation
	rm -rf $(INSTALLDIR)/asterisk/usr/lib/agi-bin
	rm -rf $(INSTALLDIR)/asterisk/var
	rm -f $(INSTALLDIR)/asterisk/usr/lib/*.a

