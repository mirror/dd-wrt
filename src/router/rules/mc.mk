export glib_cv_stack_grows=no 
export glib_cv_uscore=no 
#export ac_cv_path_GLIB_GENMARSHAL=$(STAGING_DIR_HOST)/bin/glib-genmarshal 
export ac_cv_func_mmap_fixed_mapped=yes 
export ac_cv_func_posix_getpwuid_r=no
export ac_cv_func_posix_getgrgid_r=no
#export GLIB_CONFIG=$(TOP)/glib-1.2.10-install/bin/glib-config
export GLIB_CFLAGS=-I$(TOP)/glib20/libglib/glib

mc-configure: glib20 ncurses
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
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
	cd mc && libtoolize -ci --force 
	cd mc && aclocal
	cd mc && automake --add-missing
	cd mc && autoreconf -fi 
	cd mc && ./configure --host=$(ARCH)-uclibc-linux AWK="awk" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -DSTAT_STATVFS -fcommon -I$(TOP)/glib20/gettext" \
		LDFLAGS="-L$(TOP)/glib20/gettext/.libs -lintl" \
		GLIB_CFLAGS="-I$(TOP)/_staging/usr/include/glib-2.0 -I$(TOP)/_staging/usr/lib/glib-2.0/include -L$(INSTALLDIR)/util-linux/usr/lib" \
		GLIB_LIBS="-L$(TOP)/_staging/usr/lib -lglib-2.0 -lpcre2-8" \
		GMODULE_CFLAGS="-pthread -I$(TOP)/glib20/libglib/gmodule -I$(TOP)/glib20/libglib/glib -I$(TOP)/glib20/libglib" \
		GMODULE_LIBS="-pthread $(TOP)/_staging/usr/lib -lrt -lglib-2.0" \
	--without-included-gettext \
	--with-screen=ncurses \
	--disable-nls \
	--enable-charset \
	--enable-background \
	--enable-largefile \
	--disable-vfs-sftp \
	--with-mmap \
	--with-internal-edit \
	--with-subshell \
	--without-gpm-mouse \
	--with-ncurses-includes=$(TOP)/ncurses/include \
	--with-ncurses-libs=$(TOP)/ncurses/lib \
	--without-x \
	--disable-tests \
	--disable-doxygen-doc \
	--enable-silent-rules \
	--with-homedir=/tmp/mc \
	--prefix=/usr \
	--libdir=/usr/lib \

#	--without-subshell \
#	--without-netrc \
#	--without-vfs \


#	--with-glib-prefix="$(TOP)/mc/glib" \


mc: glib20 ncurses
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
ifneq ($(CONFIG_NFS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.so*
endif
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libmount.la
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
	if test -e "mc/Makefile"; then $(MAKE) -C mc; fi


mc-install:
	if test -e "mc/Makefile"; then $(MAKE) -C mc install DESTDIR=$(INSTALLDIR)/mc; fi
	rm -rf $(INSTALLDIR)/mc/usr/share/man


mc-clean:
	if test -e "mc/Makefile"; then $(MAKE) -C mc clean; fi
