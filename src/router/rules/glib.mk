GLIB_COMP_ARGS= \
	-Ddefault_library=both \
	-Dsysprof=disabled \
	-Dtests=false \
	-Dglib_debug=disabled \
	-Dlibelf=disabled \
	-Dselinux=disabled \
	-Dlibmount=disabled \
	-Dman=false \
	-Ddtrace=false \
	-Dsystemtap=false \
	-Dgtk_doc=false \
	-Dbsymbolic_functions=false \
	-Dforce_posix_threads=true \
	-Dinstalled_tests=false \
	-Dnls=disabled \
	-Doss_fuzz=disabled \
	-Ddtrace=false \
	-Dglib_assert=false \
	-Dglib_checks=false

GLIB_STATIC_COMP_ARGS= \
	-Dsysprof=disabled \
	-Dtests=false \
	-Dglib_debug=disabled \
	-Dlibelf=disabled \
	-Dselinux=disabled \
	-Dlibmount=disabled \
	-Dman=false \
	-Ddtrace=false \
	-Dsystemtap=false \
	-Dgtk_doc=false \
	-Dbsymbolic_functions=false \
	-Dforce_posix_threads=true \
	-Dinstalled_tests=false \
	-Dnls=disabled \
	-Ddtrace=false \
	-Doss_fuzz=disabled \
	-Dglib_assert=false \
	-Dglib_checks=false

GLIB_MESON_HOST_ARGS += $(GLIB_COMP_ARGS) -Dxattr=false
GLIB_MESON_ARGS += $(GLIB_COMP_ARGS) -Dxattr=true -Db_lto=true
GLIB_STATIC_MESON_HOST_ARGS += $(GLIB_STATIC_COMP_ARGS) -Dxattr=false
GLIB_STATIC_MESON_ARGS += $(GLIB_STATIC_COMP_ARGS) -Dxattr=true -Db_lto=true

ifeq ($(ARCH),arm)
MESON_ARCH:="arm"
MESON_ENDIAN:="little"
endif
ifeq ($(ARCH),aarch64)
MESON_ARCH:="aarch64"
MESON_ENDIAN:="little"
endif
ifeq ($(ARCH),armeb)
MESON_ARCH:="arm"
MESON_ENDIAN:="big"
endif
ifeq ($(ARCH),i386)
MESON_ARCH:="x86"
MESON_ENDIAN:="little"
endif
ifeq ($(ARCH),x86_64)
MESON_ARCH:="x86_64"
MESON_ENDIAN:="little"
endif
ifeq ($(ARCH),mips)
MESON_ARCH:="mips"
MESON_ENDIAN:="big"
endif
ifeq ($(ARCH),mips64)
MESON_ARCH:="mips64"
MESON_ENDIAN:="big"
endif
ifeq ($(ARCH),mipsel)
MESON_ARCH:="mips"
MESON_ENDIAN:="little"
endif
ifeq ($(ARCH),powerpc)
MESON_ARCH:="ppc"
MESON_ENDIAN:="big"
endif
ifeq ($(ARCH),i386)
	export SUBARCH:=pc
else
ifeq ($(ARCH),x86_64)
	export SUBARCH:=pc
else
	export SUBARCH:=unknown
endif
endif


glib20-configure: libffi-configure libffi zlib-configure zlib util-linux-configure util-linux
	ln -f -r -s ${shell which $(ARCH)-openwrt-linux-gcc-ar} ${shell which $(CROSS_COMPILE)gcc}-ar
	ln -f -r -s ${shell which $(ARCH)-openwrt-linux-gcc-ranlib} ${shell which $(CROSS_COMPILE)gcc}-ranlib
	ln -f -r -s ${shell which $(ARCH)-openwrt-linux-gcc-nm} ${shell which $(CROSS_COMPILE)gcc}-nm
	echo "[binaries]" > $(TOP)/glib20/libglib/cross.txt
	echo "c = '$(subst ccache ,,$(CC))'" >> $(TOP)/glib20/libglib/cross.txt
	echo "cpp = '$(subst ccache ,,$(CXX))'" >> $(TOP)/glib20/libglib/cross.txt
	echo "ar = '$(subst ccache ,,$(CC))-ar'" >> $(TOP)/glib20/libglib/cross.txt
	echo "strip = '$(STRIP)'" >> $(TOP)/glib20/libglib/cross.txt
	echo "nm = '$(NM)'" >> $(TOP)/glib20/libglib/cross.txt
	echo "[built-in options]" >> $(TOP)/glib20/libglib/cross.txt
	echo "c_args = '$(CFLAGS) $(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF'" >> $(TOP)/glib20/libglib/cross.txt
	echo "c_link_args = '$(LDFLAGS) -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz'" >> $(TOP)/glib20/libglib/cross.txt
	echo "cpp_args = '$(CFLAGS) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib  -DNEED_PRINTF'" >> $(TOP)/glib20/libglib/cross.txt
	echo "cpp_link_args = '$(LDFLAGS) -L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz'" >> $(TOP)/glib20/libglib/cross.txt
	echo "prefix = '/usr'" >> $(TOP)/glib20/libglib/cross.txt
	echo "[host_machine]" >> $(TOP)/glib20/libglib/cross.txt
	echo "system = 'linux'" >> $(TOP)/glib20/libglib/cross.txt
	echo "cpu_family = '$(MESON_ARCH)'" >> $(TOP)/glib20/libglib/cross.txt
	echo "cpu = 'generic'" >> $(TOP)/glib20/libglib/cross.txt
	echo "endian = '$(MESON_ENDIAN)'" >> $(TOP)/glib20/libglib/cross.txt
	echo "[properties]" >> $(TOP)/glib20/libglib/cross.txt
	echo "needs_exe_wrapper = true" >> $(TOP)/glib20/libglib/cross.txt



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

	cd glib20/gettext && ./autogen.sh
	cd glib20/gettext && ./configure --enable-static --disable-shared --host=$(ARCH)-linux  LDFLAGS="$(COPTS) $(LTO) -std=gnu89 $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc " CFLAGS="$(COPTS)  $(MIPS16_OPT) $(THUMB)  -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc" CXXFLAGS="$(COPTS)  $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -fPIC -Drpl_malloc=malloc"
	make -C glib20/gettext clean all
	rm -rf $(TOP)/glib20/libglib/build
	export CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF" && \
	export CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF" && \
	export LDFLAGS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz" && \
	cd $(TOP)/glib20/libglib && meson setup --buildtype=plain --prefix=/usr --cross-file $(TOP)/glib20/libglib/cross.txt $(GLIB_MESON_ARGS) build
	export CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF" && \
	export CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)  -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF" && \
	export LDFLAGS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz" && \
	cd $(TOP)/glib20/libglib && ninja -C build
	export DESTDIR=$(TOP)/_staging && \
	cd $(TOP)/glib20/libglib && ninja -C build install


	rm -rf $(TOP)/glib20/libglib/build_static
	export CPPFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	export CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	export LDFLAGS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	cd $(TOP)/glib20/libglib && meson setup --buildtype=plain --prefix=/usr --default-library static --cross-file $(TOP)/glib20/libglib/cross.txt $(GLIB_STATIC_MESON_ARGS) build_static
	export CPPFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	export CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	export LDFLAGS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz -ffunction-sections -fdata-sections -Wl,--gc-sections" && \
	cd $(TOP)/glib20/libglib && ninja -C build_static

	export DESTDIR=$(TOP)/_staging_static && \
	cd $(TOP)/glib20/libglib && ninja -C build_static install
	rm -rf $(TOP)/_staging_static/usr/lib/*.so*

glib20: libffi zlib util-linux util-linux-install
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
	make -C glib20/gettext all


	export CPPFLAGS="$(COPTS) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib" && \
	export CFLAGS="$(COPTS) -I$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/include -I$(TOP)/zlib" && \
	export LDFLAGS="-L$(TOP)/libffi/$(ARCH)-$(SUBARCH)-linux-gnu/.libs -lffi -L$(TOP)/zlib -lz" && \
	cd $(TOP)/glib20/libglib && ninja -C build

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
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
	rm -f $(INSTALLDIR)/util-linux/lib/libfdisk.so*
	rm -f $(INSTALLDIR)/util-linux/lib/libsmartcols.so*



glib20-clean:
	make -C glib20/gettext clean
	cd $(TOP)/glib20/libglib && ninja -C build clean

glib20-install:
	make -C glib20/gettext clean
	export DESTDIR=$(INSTALLDIR)/glib20 && \
	cd $(TOP)/glib20/libglib && ninja -C build install
	rm -rf $(INSTALLDIR)/glib20/usr/bin
	rm -rf $(INSTALLDIR)/glib20/usr/include
	rm -rf $(INSTALLDIR)/glib20/usr/share
	rm -rf $(INSTALLDIR)/glib20/usr/lib/glib-2.0
	rm -rf $(INSTALLDIR)/glib20/usr/lib/pkgconfig


#	install -D glib20/libglib/glib/.libs/libglib-2.0.so.0 $(INSTALLDIR)/glib20/usr/lib/libglib-2.0.so.0
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_LIBMBIM),y)
	rm -f $(INSTALLDIR)/glib20/usr/lib/libgmodule-2.0*
endif
endif
endif

ifneq ($(CONFIG_LIBQMI),y)
ifneq ($(CONFIG_LIBMBIM),y)
	rm -f $(INSTALLDIR)/glib20/usr/lib/libgthread*
	rm -f $(INSTALLDIR)/glib20/usr/lib/libgobject*
	rm -f $(INSTALLDIR)/glib20/usr/lib/libgio*
endif
endif
	rm -rf $(INSTALLDIR)/glib20/usr/libexec
	rm -rf $(INSTALLDIR)/glib20/usr/lib/gio
	rm -f $(INSTALLDIR)/glib20/usr/lib/*.a
	rm -f $(INSTALLDIR)/glib20/usr/lib/libpcre2-16*
ifneq ($(CONFIG_FRR),y)
ifneq ($(CONFIG_MC),y)
ifneq ($(CONFIG_LIBMBIM),y)
	rm -f $(INSTALLDIR)/glib20/usr/lib/libpcre2-8*
endif
endif
endif
ifneq ($(CONFIG_LIBMBIM),y)
ifneq ($(CONFIG_MC),y)
	rm -f $(INSTALLDIR)/glib20/usr/lib/libglib-2*
endif
endif
	rm -f $(INSTALLDIR)/glib20/usr/lib/libpcre2-32*
	rm -f $(INSTALLDIR)/glib20/usr/lib/libpcre2-posix*
	-install -D glib20/gettext/gettext-runtime/intl/.libs/libintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libintl.so.8
	-install -D glib20/gettext/gettext-runtime/intl/.libs/libgnuintl.so.8 $(INSTALLDIR)/glib20/usr/lib/libgnuintl.so.8
#	install -D glib20/gettext/gettext-runtime/libasprintf/.libs/libasprintf.so.0 $(INSTALLDIR)/glib20/usr/lib/libasprintf.so.0
	-install -D glib20/gettext/gettext-runtime/src/.libs/envsubst $(INSTALLDIR)/glib20/usr/bin/envsubst
	-install -D glib20/gettext/gettext-runtime/src/.libs/gettext $(INSTALLDIR)/glib20/usr/bin/gettext
	-install -D glib20/gettext/gettext-runtime/src/.libs/ngettext $(INSTALLDIR)/glib20/usr/bin/ngettext

