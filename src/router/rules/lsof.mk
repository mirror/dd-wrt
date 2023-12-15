lsof-configure:
	cd lsof && aclocal
	cd lsof && autoconf
	cd lsof && autoheader
	cd lsof && autoreconf -vfi
	cd lsof && ./configure --prefix=/usr --libdir=/usr/lib \
		--host=$(ARCH)-linux \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF" \
		LDFLAGS="$(LDLTO)" \
		LIBTIRPC_CFLAGS="-I$(TOP)/libtirpc/tirpc" \
		LIBTIRPC_LIBS="-L$(TOP)/libtirpc/src/.libs -ltirpc"

lsof:
	$(MAKE) -C lsof all

lsof-clean:
	if test -e "lsof/Makefile"; then make -C lsof clean; fi
	@true

lsof-install:
	install -D lsof/.libs/lsof $(INSTALLDIR)/lsof/usr/sbin/lsof
#