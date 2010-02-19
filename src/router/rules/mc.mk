export glib_cv_stack_grows=no 
export glib_cv_uscore=no 
export ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOST)/bin/glib-genmarshal 
export ac_cv_func_mmap_fixed_mapped=yes 
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no
export GLIB_CONFIG=$(TOP)/glib-1.2.10-install/bin/glib-config
export GLIB_CFLAGS=-I$(TOP)/glib-1.2.10-install/include/glib-1.2

mc-configure: ncurses
#	cd mc/libiconv && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -fPIC" --enable-shared \
#	--enable-static \
#	--disable-rpath \
##	--enable-relocatable

#	cd mc/gettext && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -fPIC" --enable-shared \
#	--enable-static \
#	--disable-rpath \
#	--enable-nls \
#	--disable-java \
#	--disable-native-java \
#	--disable-openmp \
#	--with-included-gettext \
#	--without-libintl-prefix \
#	--without-libexpat-prefix \
#	--without-emacs



#	cd mc/glib && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -fPIC" --enable-shared \
#		--enable-static \
#		--enable-debug=no \
#		--with-libiconv=gnu \
#		--disable-selinux \
#    		--disable-fam \

#	cd mc/slang && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -fPIC" --enable-shared \
#		--enable-static \
#		--enable-debug=no \

	cd mc && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS) -DNEED_PRINTF" LDFLAGS="-L$(TOP)/ncurses/lib -lncurses" \
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
	--with-glib12 \
	--with-glib-prefix=$(TOP)/glib-1.2.10-install \

#	--without-subshell \
#	--without-netrc \
#	--without-vfs \


#	--with-glib-prefix="$(TOP)/mc/glib" \


mc: ncurses
	$(MAKE) -j 4 -C mc

mc-install:
	if test -e "mc/Makefile"; then $(MAKE) -C mc install DESTDIR=$(INSTALLDIR)/mc; fi


mc-clean: ncurses
	if test -e "mc/Makefile"; then $(MAKE) -C mc clean; fi
