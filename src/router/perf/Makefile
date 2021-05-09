configure:
	cd argp && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls
	make -C argp
	cd elfutils && ./configure --host=$(ARCH)-linux-elf --program-prefix= --program-suffix= --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --libexecdir=/usr/lib --sysconfdir=/etc --datadir=/usr/share --localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --disable-nls --program-prefix=eu- --disable-debuginfod --without-lzma CFLAGS="-I$(TOP)/zlib/include -I$(TOP)/perf/argp" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/perf/argp"
	make -C elfutils

all:
	make -C argp
	make -C elfutils

install:
	mkdir -p $(INSTALLDIR)
	cp -urv arm/* $(INSTALLDIR)