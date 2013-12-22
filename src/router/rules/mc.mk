export glib_cv_stack_grows=no 
export glib_cv_uscore=no 
#export ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOST)/bin/glib-genmarshal 
export ac_cv_func_mmap_fixed_mapped=yes 
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no
#export GLIB_CONFIG=$(TOP)/glib-1.2.10-install/bin/glib-config
export GLIB_CFLAGS=-I$(TOP)/glib20/libglib/glib

mc-configure: ncurses
	cd mc2/slang && ./configure --host=$(ARCH)-uclibc-linux CC="ccache $(CC)" CFLAGS="$(COPTS)  $(MIPS16_OPT) -I$(TOP)/zlib -L$(TOP)/zlib" --enable-shared \
		--enable-static \
		--enable-debug=no 
	make -C mc2/slang clean
	make -C mc2/slang

	cd mc2 && ./configure --host=$(ARCH)-uclibc-linux CC="ccache $(CC)" \
		CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -DSTAT_STATVFS -I$(TOP)/mc2/slang/src" \
		LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/mc2/slang/src/elf$(ARCH)objs -lncurses" \
		GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib" \
		GLIB_LIBS="-L$(TOP)/glib20/libglib/glib/.libs -lglib-2.0" \
		GMODULE_CFLAGS="-pthread -I$(TOP)/glib20/libglib/gmodule -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib" \
		GMODULE_LIBS="-pthread -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/glib/.libs -lrt -lglib-2.0" \
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
	--with-slang-includes=$(TOP)/mc2/slang/src \
	--with-slang-libs=$(TOP)/mc2/slang/src/elf$(ARCH)objs \
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
	$(MAKE) -j 4 -C mc2/slang
	$(MAKE) -j 4 -C mc2

mc-install:
	install -D mc2/slang/src/elf$(ARCH)objs/libslang.so.2 $(INSTALLDIR)/mc/usr/lib/libslang.so.2
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 install DESTDIR=$(INSTALLDIR)/mc; fi
	rm -rf $(INSTALLDIR)/mc/usr/share/man


mc-clean:
	if test -e "mc2/slang/Makefile"; then $(MAKE) -C mc2/slang clean; fi
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 clean; fi
