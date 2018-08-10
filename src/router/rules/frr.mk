frr-configure: ncurses json-c
	cd frr && autoreconf --force --install
	cd frr/readline && ./configure --host=$(ARCH)-uclibc-linux --prefix=/usr  CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -fno-strict-aliasing"
	$(MAKE) -C frr/readline clean all
	rm -rf frr/build
	-mkdir -p frr/build
	cd frr/build && ../configure CC="" CFLAGS="" LDFLAGS="" --disable-eigrpd --disable-ldpd --disable-pbrd --disable-rfptest --disable-ssd  --disable-doc --enable-clippy-only --disable-zeromq --enable-opaque-lsa --disable-nhrpd --enable-ospf-te --disable-ospfclient --enable-multipath=32  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd --enable-pie=no
	make -C frr/build lib/clippy	
	-mkdir -p frr/build/hosttools/lib
	cd frr/build && cp -vR lib/* hosttools/lib
	cd frr/build && ../configure --host=$(ARCH)-uclibc-linux --localstatedir=/var/run  --libdir=/usr/tmp  --disable-eigrpd --disable-pbrd --disable-ldpd  --disable-rfptest --disable-ssd --disable-doc --disable-zeromq --enable-opaque-lsa --disable-nhrpd --enable-ospf-te --disable-ospfclient --enable-multipath=32  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd --enable-pie=no --with-libreadline=$(TOP)/frr/readline CFLAGS="-fno-strict-aliasing -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/frr -Drpl_malloc=malloc -Drpl_realloc=realloc -I$(TOP)/_staging/usr/include -I$(TOP)/frr/build" LDFLAGS="-L$(TOP)/frr/readline -L$(TOP)/ncurses/lib -lncurses -L$(TOP)/json-c/.libs -ljson-c" 

frr: ncurses json-c
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
	-if test -e "frr/Makefile"; then $(MAKE) -C frr clean; fi
