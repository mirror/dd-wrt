rpcbind-configure: libtirpc
	cd rpcbind && ./autogen.sh
	cd rpcbind && ./configure --libdir=/usr/lib --host=$(ARCH)-linux \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc" \
		LDFLAGS="$(LDLTO) -L$(TOP)/libtirpc/src/.libs" \
		TIRPC_CFLAGS="-I$(TOP)/libtirpc -I$(TOP)/libtirpc/tirpc" \
		TIRPC_LIBS="-L$(TOP)/libtirpc/src/.libs -ltirpc" \
		--prefix=/usr --with-systemdsystemunitdir=no

rpcbind: libtirpc
	make -C rpcbind

rpcbind-clean:
	make -C rpcbind clean

rpcbind-install:
	make -C rpcbind install DESTDIR=$(INSTALLDIR)/rpcbind
	find $(INSTALLDIR)/rpcbind -name *.la -delete
	rm -rf $(INSTALLDIR)/rpcbind/usr/include
	rm -rf $(INSTALLDIR)/rpcbind/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/rpcbind/usr/share
	rm -rf $(INSTALLDIR)/rpcbind/var
	rm -f $(INSTALLDIR)/rpcbind/usr/lib/*.a
	rm -f $(INSTALLDIR)/rpcbind/usr/lib/*.la
