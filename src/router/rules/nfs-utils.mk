lvm2-configure:
	cd lvm2 && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" \
		BLKID_CFLAGS="-L$(TOP)/e2fsprogs/lib/blkid" \
		BLKID_LIBS="-L$(TOP)/e2fsprogs/lib/blkid -lblkid"

lvm2:
	make -C lvm2 device-mapper

lvm2-clean:
	make -C lvm2 clean

lvm2-install:


keyutils:
	make -C keyutils

keyutils-clean:
	make -C keyutils clean

keyutils-install:
	make -C keyutils install DESTDIR=$(INSTALLDIR)/keyutils

nfs-utils-configure: libtirpc lvm2
	cd nfs-utils && ./autogen.sh
	cd nfs-utils && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --with-tirpcinclude=$(TOP)/libtirpc/tirpc --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(TOP)/lvm2/libdm  -I$(TOP)/keyutils" \
		LDFLAGS="-L$(TOP)/libtirpc/src/.libs  -L$(TOP)/libevent/.libs -L$(TOP)/lvm2/libdm/ioctl -L$(TOP)/keyutils" \
		--with-rpcgen=internal --disable-uuid --disable-gssapi --disable-static --prefix=/usr \
		--disable-gss --disable-nfsdcltrack

nfs-utils: libtirpc lvm2
	make -C nfs-utils

nfs-utils-clean:
	make -C nfs-utils clean

nfs-utils-install:
	make -C nfs-utils install DESTDIR=$(INSTALLDIR)/nfs-utils
	find $(INSTALLDIR)/nfs-utils -name *.la -delete
	rm -rf $(INSTALLDIR)/nfs-utils/usr/include
	rm -rf $(INSTALLDIR)/nfs-utils/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/nfs-utils/usr/share
	rm -rf $(INSTALLDIR)/nfs-utils/usr/var
	rm -f $(INSTALLDIR)/nfs-utils/usr/lib/*.a
	rm -f $(INSTALLDIR)/nfs-utils/usr/lib/*.la
