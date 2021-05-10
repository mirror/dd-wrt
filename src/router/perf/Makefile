all:
	make -C argp
	make -C elfutils
	make -C elfutils install DESTDIR=$(TOP)/_staging
	make -C ../libunwind install DESTDIR=$(TOP)/_staging
	make -C tools/perf EXTRA_CFLAGS="-fcommon -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_SETNS_SUPPORT -DNEED_PRINTF -D_GNU_SOURCE -D__UCLIBC__ -I$(TOP)/perf -I$(TOP)/_staging/usr/include -I$(TOP)/zlib/include" \
		LDFLAGS="-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -lz -lelf" \
		CROSS_COMPILE=$(ARCH)-linux- \
		LIBDW_DIR=$(TOP)/_staging/usr \
		NO_LIBPERL=1 \
		NO_LIBPYTHON=1 \
		NO_NEWT=1 \
		NO_LZMA=1 \
		NO_BACKTRACE=1 \
		NO_LIBNUMA=1 \
		NO_GTK2=1 \
		NO_LIBAUDIT=1 \
		NO_LIBCRYPTO=1 \
		NO_LIBUNWIND=1 \
		NO_LIBZSTD=1 \
		NO_LIBCAP=1 \
		WERROR=0 \
		V=1 \
		prefix=/usr
		
clean:
	make -C argp clean
	make -C elfutils clean
	make -C tools/perf EXTRA_CFLAGS="-fcommon -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_SETNS_SUPPORT -DNEED_PRINTF -D_GNU_SOURCE -D__UCLIBC__ -I$(TOP)/perf -I$(TOP)/_staging/usr/include -I$(TOP)/zlib/include" \
		LDFLAGS="-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -lz -lelf" \
		CROSS_COMPILE=$(ARCH)-linux- \
		LIBDW_DIR=$(TOP)/_staging/usr \
		NO_LIBPERL=1 \
		NO_LIBPYTHON=1 \
		NO_NEWT=1 \
		NO_LZMA=1 \
		NO_BACKTRACE=1 \
		NO_LIBNUMA=1 \
		NO_GTK2=1 \
		NO_LIBAUDIT=1 \
		NO_LIBCRYPTO=1 \
		NO_LIBUNWIND=1 \
		NO_LIBZSTD=1 \
		NO_LIBCAP=1 \
		WERROR=0 \
		V=1 \
		prefix=/usr \
		clean



configure:
	cd argp && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls
	make -C argp
	cd elfutils && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --disable-nls --program-prefix=eu- --disable-debuginfod --without-lzma CFLAGS="-I$(TOP)/zlib/include -I$(TOP)/perf/argp" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/perf/argp"
	make -C elfutils


install:
	mkdir -p $(INSTALLDIR)
	make -C elfutils install DESTDIR=$(INSTALLDIR)
	make -C tools/perf EXTRA_CFLAGS="-fcommon -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_SETNS_SUPPORT -DNEED_PRINTF -D_GNU_SOURCE -D__UCLIBC__ -I$(TOP)/perf -I$(TOP)/_staging/usr/include -I$(TOP)/zlib/include" \
		LDFLAGS="-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -lz -lelf" \
		CROSS_COMPILE=$(ARCH)-linux- \
		LIBDW_DIR=$(TOP)/_staging/usr \
		NO_LIBPERL=1 \
		NO_LIBPYTHON=1 \
		NO_NEWT=1 \
		NO_LZMA=1 \
		NO_BACKTRACE=1 \
		NO_LIBNUMA=1 \
		NO_GTK2=1 \
		NO_LIBAUDIT=1 \
		NO_LIBCRYPTO=1 \
		NO_LIBUNWIND=1 \
		NO_LIBZSTD=1 \
		NO_LIBCAP=1 \
		WERROR=0 \
		V=1 \
		prefix=/usr \
		install-bin DESTDIR=$(INSTALLDIR)
	rm -rf $(INSTALLDIR)/etc
	rm -f $(INSTALLDIR)/usr/bin/trace
	rm -rf $(INSTALLDIR)/usr/include
	rm -rf $(INSTALLDIR)/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/usr/lib/perf
	rm -rf $(INSTALLDIR)/usr/share

