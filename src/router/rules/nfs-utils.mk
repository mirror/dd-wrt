nfs-utils-configure: libtirpc lvm2 keyutils krb5 libevent sqlite
	cd nfs-utils && ./autogen.sh
	cd nfs-utils && ./configure \
		--enable-fast-install \
		--with-sysroot=yes \
		--libdir=/usr/lib \
		--libexecdir=/usr/libexec \
		--with-tirpcinclude=$(TOP)/libtirpc/tirpc --host=$(ARCH)-linux \
		--with-rpcgen=internal \
		--disable-caps \
		--disable-uuid \
		--disable-gssapi \
		--disable-static \
		--prefix=/usr \
		--enable-gss \
		--disable-nfsdcltrack \
		--disable-nfsdcld \
		--with-krb5=yes \
		TIRPC_CFLAGS="-I$(TOP)/libtirpc  -I$(TOP)/libtirpc/tirpc" \
		KRBCFLAGS="-I$(TOP)/krb5/src/include" \
		KRBLDFLAGS="-L$(TOP)/krb5/src/lib" \
		KRBLIBS="-lkrb5 -lk5crypto -lkrb5support -lcom_err" \
		GSSKRB_LIBS="-lgssapi_krb5 -lgssrpc" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO)  -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64  -DNEED_PRINTF -fcommon -I$(TOP)/sqlite -I$(TOP)/libevent -I$(TOP)/libevent/include -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc -I$(TOP)/lvm2/libdm  -I$(TOP)/keyutils -I$(TOP)/util-linux/libuuid/src -I$(TOP)/util-linux/libmount/src -D_GNU_SOURCE" \
		LDFLAGS="$(LDLTO) -L$(TOP)/libtirpc/src/.libs  -L$(TOP)/libevent/.libs -L$(TOP)/lvm2/libdm/ioctl -L$(TOP)/sqlite/.libs -L$(TOP)/keyutils -L$(TOP)/krb5/src/lib -L$(TOP)/util-linux/.libs/" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" 


nfs-utils: libtirpc lvm2 keyutils krb5 sqlite
	install -D nfs-utils/config/nfs.webnas httpd/ej_temp/04nfs.webnas
	make -C nfs-utils

nfs-utils-clean:
	make -C nfs-utils clean

nfs-utils-install:
	make -C nfs-utils install DESTDIR=$(INSTALLDIR)/nfs-utils
	find $(INSTALLDIR)/nfs-utils -name *.la -delete
	rm -rf $(INSTALLDIR)/nfs-utils/usr/include
	rm -rf $(INSTALLDIR)/nfs-utils/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/nfs-utils/usr/share
	rm -rf $(INSTALLDIR)/nfs-utils/usr/bin
	rm -rf $(INSTALLDIR)/nfs-utils/var
	rm -f $(INSTALLDIR)/nfs-utils/usr/lib/*.a
	rm -f $(INSTALLDIR)/nfs-utils/usr/lib/*.la
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/blkmapd
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/mountstats
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/rpcdebug
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/nfsdclnts
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/nfsiostat
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/rpcctl
	rm -f $(INSTALLDIR)/nfs-utils/usr/sbin/start-statd
	mkdir -p $(INSTALLDIR)/nfs-utils/etc
	rm -f $(INSTALLDIR)/nfs-utils/etc/exports
	-cd $(INSTALLDIR)/nfs-utils/etc && ln -s /tmp/exports exports
	install -D nfs-utils/config/nfs.webnas $(INSTALLDIR)/nfs-utils/etc/config/04nfs.webnas
	install -D nfs-utils/config/nfs.nvramconfig $(INSTALLDIR)/nfs-utils/etc/config/nfs.nvramconfig
