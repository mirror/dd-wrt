ubi-utils-configure: zlib
	cd ubi-utils && ./autogen.sh
	cd ubi-utils && ./configure --prefix=/usr --host=$(ARCH)-linux --without-tests --without-crypto \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) -I$(TOP)/lzo/include -L$(TOP)/lzo/src/.libs -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LZO_CFLAGS="$(LTO) -I$(TOP)/lzo/include" \
		LZO_LIBS="$(LDLTO) -L$(TOP)/lzo/src/.libs -llzo2" \
		ZLIB_CFLAGS=" $(LTO) -I$(TOP)/zlib/include" \
		ZLIB_LIBS=" $(LDLTO) -L$(TOP)/zlib -lz" \
		ZSTD_CFLAGS="$(LTO) -I$(TOP)/zstd/lib" \
		ZSTD_LIBS="$(LDLTO) -L$(TOP)/zstd/lib -lzstd" \
		UUID_CFLAGS="-I$(TOP)/$(ARCH)-uclibc/install/util-linux/usr/include/uuid" \
		UUID_LIBS="$(TOP)/util-linux/.libs/libuuid.a" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

ubi-utils: zlib
	$(MAKE) -C util-linux
	$(MAKE) -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
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
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
	$(MAKE) -C ubi-utils

ubi-utils-clean:
	$(MAKE) -C ubi-utils clean

ubi-utils-install:
	$(MAKE) -C util-linux
	$(MAKE) -C util-linux install DESTDIR=$(INSTALLDIR)/util-linux
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
ifneq ($(CONFIG_ZFS),y)
ifneq ($(CONFIG_WEBSERVER),y)
ifneq ($(CONFIG_E2FSPROGS),y)
	rm -f $(INSTALLDIR)/util-linux/usr/lib/libblkid.so*
endif
endif
endif
endif
endif
endif
endif
	$(MAKE) -C ubi-utils install DESTDIR=$(INSTALLDIR)/ubi-utils
	rm -rf $(INSTALLDIR)/ubi-utils/usr/share
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
