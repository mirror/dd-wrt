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
