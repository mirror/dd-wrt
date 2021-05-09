all:
	make -C argp
	make -C elfutils
	make -C elfutils install DESTDIR=$(TOP)/_staging
	make -C ../libunwind install DESTDIR=$(TOP)/_staging
	make -C $(LINUXDIR)/tools/perf EXTRA_CFLAGS="-fcommon -DNEED_PRINTF -D_GNU_SOURCE -D__UCLIBC__ -I$(TOP)/perf -I$(TOP)/_staging/usr/include -I$(TOP)/zlib/include" LDFLAGS="-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -lz -lelf" CROSS_COMPILE=$(ARCH)-linux- LIBDW_DIR=$(TOP)/_staging/usr 

configure:
	cd argp && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls
	make -C argp
	cd elfutils && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --disable-nls --program-prefix=eu- --disable-debuginfod --without-lzma CFLAGS="-I$(TOP)/zlib/include -I$(TOP)/perf/argp" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/perf/argp"
	make -C elfutils


install:
	mkdir -p $(INSTALLDIR)
	make -C elfutils install DESTDIR=$(INSTALLDIR)
	make -C $(LINUXDIR)/tools/perf EXTRA_CFLAGS="-fcommon -DNEED_PRINTF -D_GNU_SOURCE -D__UCLIBC__ -I$(TOP)/perf -I$(TOP)/_staging/usr/include -I$(TOP)/zlib/include" LDFLAGS="-L$(TOP)/_staging/usr/lib -L$(TOP)/zlib -lz -lelf" CROSS_COMPILE=$(ARCH)-linux- LIBDW_DIR=$(TOP)/_staging/usr  install-bin DESTDIR=$(INSTALLDIR)/usr
	rm -rf $(INSTALLDIR)/usr/etc
	rm -f $(INSTALLDIR)/usr/bin/trace
	rm -rf $(INSTALLDIR)/usr/include
	rm -rf $(INSTALLDIR)/usr/lib/pkgconfig
	