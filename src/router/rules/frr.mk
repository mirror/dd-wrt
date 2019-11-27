libcap:
	make -C libcap

libcap-clean:
	make -C libcap clean

libcap-install:
	make -C libcap install DESTDIR=$(INSTALLDIR)/libcap




frr-configure: ncurses json-c readline libyang libcap
	cd frr && autoreconf --force --install
	rm -rf frr/build
	-mkdir -p frr/build
	cd frr/build && ../configure CC="" CFLAGS="" LDFLAGS="" LD="" --with-vtysh-pager=less --disable-eigrpd --disable-ldpd --enable-shared --disable-pbrd --disable-rfptest --disable-ssd  --disable-doc --enable-clippy-only --enable-shared --disable-zeromq --enable-opaque-lsa --disable-nhrpd --enable-ospf-te --disable-ospfclient --enable-multipath=32  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd --disable-staticd --disable-bfdd --disable-babeld --enable-pie=no
	make -C frr/build lib/clippy
	-mkdir -p frr/build/hosttools/lib
	cd frr/build && cp -vR lib/* hosttools/lib
	cd frr/build && ../configure \
		--host=$(ARCH)-uclibc-linux \
		--localstatedir=/var/run \
		--libdir=/usr/tmp \
		--with-vtysh-pager=less  \
		--disable-eigrpd \
		--disable-pbrd \
		--disable-ldpd  \
		--disable-rfptest \
		--disable-ssd \
		--enable-shared \
		--disable-doc \
		--disable-zeromq \
		--disable-backtrace \
		--enable-opaque-lsa \
		--disable-nhrpd \
		--enable-ospf-te --disable-ospfclient --enable-multipath=32  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d \
		--enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd \
		--disable-staticd --disable-bfdd --disable-babeld --enable-pie=no --with-libreadline=$(TOP)/readline \
		CFLAGS="-fno-strict-aliasing -I$(TOP)/libcap/libcap/include -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/ -Drpl_malloc=malloc -Drpl_realloc=realloc -I$(TOP)/_staging/usr/include -I$(TOP)/frr/build -I$(TOP)/libyang/src" \
		CPPFLAGS="-fno-strict-aliasing -I$(TOP)/libcap/libcap/include -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/ -Drpl_malloc=malloc -Drpl_realloc=realloc -I$(TOP)/_staging/usr/include -I$(TOP)/frr/build -I$(TOP)/libyang/src" \
		LDFLAGS="-L$(TOP)/readline/shlib -L$(TOP)/ncurses/lib -lncurses -L$(TOP)/json-c/.libs -ljson-c -L$(TOP)/libyang -lyang -L$(TOP)/pcre/.libs -lpcre -L$(TOP)/libcap/libcap -lcap" \
		LIBYANG_CFLAGS="-I$(TOP)/libyang/src" \
		LIBYANG_LIBS="-L$(TOP)/libyang -lyang -L$(TOP)/pcre/.libs -lpcre"

frr: ncurses json-c libyang libcap
	make -C frr/build

frr-install:
	make -C frr/build DESTDIR=$(INSTALLDIR)/frr install
	mkdir -p $(INSTALLDIR)/frr/usr/lib
	-cp -urv $(INSTALLDIR)/frr/usr/tmp/* $(INSTALLDIR)/frr/usr/lib
	rm -rf $(INSTALLDIR)/frr/usr/tmp 

	rm -rf $(INSTALLDIR)/frr/tmp
	rm -rf $(INSTALLDIR)/frr/usr/info
	rm -rf $(INSTALLDIR)/frr/usr/share
	rm -rf $(INSTALLDIR)/frr/usr/include
	rm -rf $(INSTALLDIR)/frr/usr/etc
	rm -rf $(INSTALLDIR)/frr/usr/man
	rm -f $(INSTALLDIR)/frr/usr/lib/*.a
	rm -f $(INSTALLDIR)/frr/usr/lib/*.la
	rm -f $(INSTALLDIR)/frr/usr/bin/bgp_btoa
	rm -f $(INSTALLDIR)/frr/usr/bin/test_igmpv3_join
	mkdir -p $(INSTALLDIR)/frr/usr/bin

frr-clean:
	-if test -e "frr/build/Makefile"; then $(MAKE) -C frr/build clean; fi
