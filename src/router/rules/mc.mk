export glib_cv_stack_grows=no 
export glib_cv_uscore=no 
#export ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOST)/bin/glib-genmarshal 
export ac_cv_func_mmap_fixed_mapped=yes 
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no
#export GLIB_CONFIG=$(TOP)/glib-1.2.10-install/bin/glib-config
export GLIB_CFLAGS=-I$(TOP)/glib20/libglib/glib

mc-configure: ncurses
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	cd mc2/slang && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS)  $(MIPS16_OPT) -I$(TOP)/zlib -L$(TOP)/zlib -L$(INSTALLDIR)/util-linux/usr/lib" --enable-shared \
		--enable-static \
		--without-png \
		--libdir=/usr/lib \
		--enable-debug=no 
	make -C mc2/slang clean
	make -C mc2/slang

	cd mc2 && ./configure --host=$(ARCH)-uclibc-linux AWK="awk" \
		CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF -DSTAT_STATVFS -I$(TOP)/mc2/slang/src -I$(TOP)/ncurses/include" \
		LDFLAGS="-L$(TOP)/ncurses/lib -L$(TOP)/mc2/slang/src/elf$(ARCH)objs -lncurses" \
		GLIB_CFLAGS="-I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib -L$(INSTALLDIR)/util-linux/usr/lib" \
		GLIB_LIBS="-L$(TOP)/glib20/libglib/glib/.libs -lglib-2.0" \
		GMODULE_CFLAGS="-pthread -I$(TOP)/glib20/libglib/gmodule -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib" \
		GMODULE_LIBS="-pthread -L$(TOP)/glib20/libglib/gmodule/.libs -L$(TOP)/glib20/libglib/glib/.libs -lrt -lglib-2.0" \
	--with-included-gettext \
	--with-ncurses \
	--with-screen=ncurses \
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
	--disable-tests \
	--disable-doxygen-doc \
	--enable-silent-rules \
	--with-homedir=/tmp/mc \
	--disable-vfs-sftp \
	--prefix=/usr \
	--libdir=/usr/lib \

#	--without-subshell \
#	--without-netrc \
#	--without-vfs \


#	--with-glib-prefix="$(TOP)/mc/glib" \


mc: ncurses
	make -C util-linux
	make -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
	mkdir -p $(INSTALLDIR)/util-linux/usr/lib
	-cp -urv $(INSTALLDIR)/util-linux/usr/tmp/* $(INSTALLDIR)/util-linux/usr/lib
	rm -rf $(INSTALLDIR)/util-linux/usr/tmp 
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libuuid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.la
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	$(MAKE) -j 4 -C mc2/slang
	$(MAKE) -j 4 -C mc2
	rm -rf $(INSTALLDIR)/util-linux/usr/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/bin
	rm -rf $(INSTALLDIR)/util-linux/bin
	rm -rf $(INSTALLDIR)/util-linux/sbin
	rm -rf $(INSTALLDIR)/util-linux/usr/share
	rm -rf $(INSTALLDIR)/util-linux/usr/include
	rm -rf $(INSTALLDIR)/util-linux/usr/lib/pkgconfig
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.a
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
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
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*



mc-install:
	install -D mc2/slang/src/elf$(ARCH)objs/libslang.so.2 $(INSTALLDIR)/mc/usr/lib/libslang.so.2
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 install DESTDIR=$(INSTALLDIR)/mc; fi
	rm -rf $(INSTALLDIR)/mc/usr/share/man


mc-clean:
	if test -e "mc2/slang/Makefile"; then $(MAKE) -C mc2/slang clean; fi
	if test -e "mc2/Makefile"; then $(MAKE) -C mc2 clean; fi
