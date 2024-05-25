editline-configure:
	cd editline && ./configure --host=$(ARCH)-linux-uclibc --prefix=/usr --libdir=/usr/lib \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/ncurses/include" \
	LDFLAGS="-L$(TOP)/ncurses/lib"

editline:
	make -C editline

editline-install:
	make -C editline install DESTDIR=$(INSTALLDIR)/editline
	rm -rf $(INSTALLDIR)/editline/usr/share
	rm -rf $(INSTALLDIR)/editline/usr/include
	rm -rf $(INSTALLDIR)/editline/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/editline/usr/lib/*.a
	rm -f $(INSTALLDIR)/editline/usr/lib/*.la


asterisk-configure: util-linux-configure jansson editline zlib sqlite
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -f asterisk/menuselect.makeopts && \
	cd asterisk && ./configure --host=$(ARCH)-linux-uclibc \
	--libdir=/usr/lib \
	--without-cap \
	--without-curl \
	--without-curses \
	--with-gsm=internal \
	--with-ilbc=internal \
	--without-gtk \
	--without-gtk2 \
	--without-isdnnet \
	--without-kde \
	--without-misdn \
	--without-nbs \
	--with-ncurses="$(TOP)/ncurses" \
	--with-crypto="$(TOP)/openssl" \
	--without-netsnmp \
	--without-newt \
	--without-odbc \
	--without-ogg \
	--without-osptk \
	--without-pri \
	--without-qt \
	--without-pwlib \
	--without-sqlite \
	--without-mysql \
	--without-mysqlclient \
	--without-radius \
	--without-sdl \
	--without-spandsp \
	--without-suppserv \
	--without-tds \
	--without-termcap \
	--without-tinfo \
	--without-vorbis \
	--without-tonezone \
	--without-vpb \
	--with-z="$(TOP)/zlib" \
	--disable-xmldoc \
	--without-libxml2 \
	--without-dahdi \
	--without-gnutls \
	--without-iksemel \
	--with-uuid=$(INSTALLDIR)/util-linux/usr \
	ac_cv_header_locale_h=yes \
	pkg_cv_SYSTEMD_LIBS="" \
	SYSTEMD_CFLAGS=" " \
	SYSTEMD_INCLUDE=" " \
	SYSTEMD_LIB=' ' \
	SYSTEMD_LIBS=' ' \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -Wno-int-conversion -I$(TOP)/openssl/include -L$(TOP)/openssl -L$(TOP)/sqlite/.libs -I$(INSTALLDIR)/util-linux/usr/include -L$(TOP)/util-linux/.libs -DLOW_MEMORY -DNEED_PRINTF" \
	CXXFLAGS="$(COPTS) $(MIPS16_OPT) -Wno-int-conversion -I$(TOP)/openssl/include -L$(TOP)/openssl -L$(TOP)/sqlite/.libs -I$(INSTALLDIR)/util-linux/usr/include -L$(TOP)/util-linux/.libs -DLOW_MEMORY -DNEED_PRINTF" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -Wno-int-conversion -L$(TOP)/sqlite/.libs -DLOW_MEMORY -DNEED_PRINTF -L$(TOP)/util-linux/.libs" \
	LDFLAGS="-L$(TOP)/util-linux/.libs" \
	SQLITE3_LIB="-L$(TOP)/sqlite/.libs -lsqlite3" \
	SQLITE3_INCLUDE="-I$(TOP)/sqlite -I$(TOP)/openssl/include -L$(TOP)/openssl" \
	LIBUUID_LIB="-L$(TOP)/util-linux/.libs -luuid" \
	LIBUUID_INCLUDE="-I $(INSTALLDIR)/util-linux/usr/include" \
	NCURSES_CFLAGS="-I$(TOP)/ncurses/include" \
	NCURSES_LIB="-L$(TOP)/ncurses/lib -lncurses" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto" \
	JANSSON_CFLAGS="-I$(TOP)/jansson/src" \
	JANSSON_LIBS="-L$(TOP)/jansson/src/.libs -ljansson -L$(TOP)/sqlite/.libs -lsqlite3 -L$(TOP)/openssl" \
	LIBEDIT_CFLAGS="-I$(TOP)/editline/src" \
	LIBEDIT_LIBS="-L$(TOP)/editline/src/.libs -ledit -L$(TOP)/ncurses/lib -lncurses"
	-cd chan_dongle && aclocal && autoconf && automake -a && cd ..
	cd chan_dongle && ./configure  ac_cv_header_locale_h=yes --host=$(ARCH)-linux-uclibc --libdir=/usr/lib --with-asterisk=$(TOP)/asterisk/include DESTDIR=$(INSTALLDIR)/asterisk/usr/lib/asterisk/modules CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/glib20/libiconv/include -DASTERISK_VERSION_NUM=13000 -DLOW_MEMORY -D_XOPEN_SOURCE=600"

asterisk-clean:
	$(MAKE) -C asterisk clean
#	$(MAKE) -C chan_dongle clean

asterisk: jansson zlib
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
	-make -C asterisk \
		include/asterisk/version.h \
		include/asterisk/buildopts.h defaults.h \
		makeopts.embed_rules
	-ASTCFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -I$(TOP)/ncurses/include -I$(INSTALLDIR)/util-linux/usr/include -I$(TOP)/openssl/include -I$(TOP)/sqlite" \
	ASTLDFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -L$(TOP)/ncurses/lib -L$(TOP)/openssl -L$(TOP)/sqlite/.libs" \
	make -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		all
	-make -C asterisk

	-make -C asterisk \
		include/asterisk/version.h \
		include/asterisk/buildopts.h defaults.h \
		makeopts.embed_rules
	-ASTCFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -I$(TOP)/ncurses/include -I$(INSTALLDIR)/util-linux/usr/include -I$(TOP)/openssl/include -I$(TOP)/sqlite" \
	ASTLDFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -L$(TOP)/ncurses/lib -L$(TOP)/openssl -L$(TOP)/sqlite/.libs" \
	make -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		all
	-make -C asterisk
#	make -C chan_dongle
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_ZFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*



asterisk-install:
	chmod 700 asterisk/build_tools/install_subst
	-ASTCFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -I$(TOP)/ncurses/include -I$(INSTALLDIR)/util-linux/usr/include -I$(TOP)/openssl/include -I$(TOP)/sqlite" \
	ASTLDFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -L$(TOP)/ncurses/lib -L$(TOP)/openssl -L$(TOP)/sqlite/.libs" \
	$(MAKE) -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		DESTDIR=$(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk \
		install samples
	-ASTCFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -I$(TOP)/ncurses/include -I$(INSTALLDIR)/util-linux/usr/include -I$(TOP)/openssl/include -I$(TOP)/sqlite" \
	ASTLDFLAGS="$(COPTS) $(MIPS16_OPT) -DLOW_MEMORY -DNEED_PRINTF -fPIC -L$(TOP)/ncurses/lib -L$(TOP)/openssl -L$(TOP)/sqlite/.libs" \
	$(MAKE) -C asterisk \
		ASTVARLIBDIR="/usr/lib/asterisk" \
		NOISY_BUILD="1" \
		DEBUG="" \
		OPTIMIZE="" \
		DESTDIR=$(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk \
		adsi
#	sed 's|/var/lib/asterisk|/usr/lib/asterisk|g' $(INSTALLDIR)/asterisk/etc/asterisk/musiconhold.conf
	$(INSTALL_DIR) -p $(INSTALLDIR)/asterisk/etc/asterisk
	for f in asterisk extensions features \
		logger manager modules \
		sip sip_notify rtp; do \
		$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/$$f.conf $(INSTALLDIR)/asterisk/etc/asterisk/ ; \
	done
	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	for f in app_dial app_echo app_playback app_macro \
		chan_sip \
		codec_ulaw codec_gsm \
		format_gsm format_pcm format_wav format_wav_gsm \
		pbx_config \
		func_strings func_timeout func_callerid func_periodic_hook; do \
		$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/$$f.so $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/ ; \
	done
	rm -rf $(INSTALLDIR)/asterisk/usr/sbin
	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/sbin
	$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/sbin/asterisk $(INSTALLDIR)/asterisk/usr/sbin/
	$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/sbin/astgenkey $(INSTALLDIR)/asterisk/usr/sbin/
	$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/sbin/astcanary $(INSTALLDIR)/asterisk/usr/sbin/
	ln -s asterisk $(INSTALLDIR)/asterisk/usr/sbin/rasterisk
	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/etc/asterisk
	$(INSTALL_DATA) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/voicemail.conf $(INSTALLDIR)/asterisk/etc/asterisk/
	$(INSTALL_DIR)  $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/*voicemail.so $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/res_adsi.so $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/

	mv $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/*.so* $(INSTALLDIR)/asterisk/usr/lib/

#	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/sounds/
#	$(CP) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/sounds/en/vm-*.gsm $(INSTALLDIR)/asterisk/usr/lib/asterisk/sounds/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/etc/asterisk
	$(INSTALL_DATA) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/iax.conf $(INSTALLDIR)/asterisk/etc/asterisk/
	$(INSTALL_DATA) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/iaxprov.conf $(INSTALLDIR)/asterisk/etc/asterisk/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/chan* $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/app_system.so $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/format* $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/etc/asterisk
	$(INSTALL_DATA) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/cdr*.conf $(INSTALLDIR)/asterisk/etc/asterisk/
	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/*cdr*.so $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/

	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/etc/asterisk
	$(INSTALL_DATA) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/etc/asterisk/musiconhold.conf $(INSTALLDIR)/asterisk/etc/asterisk/
	$(INSTALL_DIR) $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules
	$(INSTALL_BIN) $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk/usr/lib/asterisk/modules/res* $(INSTALLDIR)/asterisk/usr/lib/asterisk/modules/
	rm -rf $(TOP)/$(ARCH)-uclibc/tmp/$(ARCHITECTURE)/asterisk
#	make -C chan_dongle install
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libfdisk*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libsmartcols*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_ZABBIX),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_ZFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*


