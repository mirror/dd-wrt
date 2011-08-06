export glib_cv_stack_grows=no 
export glib_cv_uscore=no 
#export ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOST)/bin/glib-genmarshal 
export ac_cv_func_mmap_fixed_mapped=yes 
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no
#export GLIB_CONFIG=$(TOP)/glib-1.2.10-install/bin/glib-config
export GLIB_CFLAGS=-I$(TOP)/usbip/libglib/glib

mc-configure: ncurses

	cd usbip/libiconv && ./configure --enable-static --disable-shared --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/libiconv clean all
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*.so*
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*.la*
	rm -f $(TOP)/usbip/libiconv/lib/.libs/*la*

	cd usbip/gettext && ./configure --enable-static --disable-shared --disable-openmp --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc"
	make -C usbip/gettext clean all

	cd usbip/libglib && ./configure --enable-shared --disable-static --host=$(ARCH)-linux CC=$(CC) CFLAGS="$(COPTS) -DNEED_PRINTF -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc -I$(TOP)/usbip/gettext/gettext-runtime/intl  -I$(TOP)/usbip/libiconv/include -L$(TOP)/usbip/libiconv/lib/.libs -L$(TOP)/usbip/gettext/gettext-runtime/intl/.libs" --with-libiconv=gnu
	make -C usbip/libglib clean all

	cd mc2/slang && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -I$(TOP)/zlib -L$(TOP)/zlib -fPIC" --enable-shared \
		--enable-static \
		--enable-debug=no 
	make -C mc2/slang

	cd mc2 && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/usbip/libglib/glib -I$(TOP)/usbip/libglib -I$(TOP)/mc2/slang/src" LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/mc2/slang/src/elf$(ARCH)objs -L$(TOP)/usbip/libglib/glib/.libs -lncurses" \
	--with-included-gettext \
	--with-ncurses \
	--without-sco \
	--without-sunos-curses \
	--without-osf1-curses \
	--without-vcurses \
	--without-gpm-mouse \
	--without-hsc \
	--without-termnet \
	--without-debug \
	--without-efence \
	--without-terminfo \
	--without-termcap \
	--without-slang \
	--without-ext2undel \
	--without-catgets \
	--without-x \
	--without-tk \
	--without-xview \
	--disable-glibtest \
	--prefix=/usr \

#	--without-subshell \
#	--without-netrc \
#	--without-vfs \


#	--with-glib-prefix="$(TOP)/mc/glib" \


mc: ncurses
	$(MAKE) -j 4 -C mc2

mc-install:
	install -D usbip/libglib/glib/.libs/libglib-2.0.so.0 $(INSTALLDIR)/mc/usr/lib/libglib-2.0.so.0
	install -D mc2/slang/src/elf$(ARCH)objs/libslang.so.2 $(INSTALLDIR)/mc/usr/lib/libslang.so.2
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 install DESTDIR=$(INSTALLDIR)/mc; fi


mc-clean: ncurses
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 clean; fi
