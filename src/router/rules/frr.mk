libcap:
	make -C libcap

libcap-clean:
	make -C libcap clean

libcap-install:
	make -C libcap install DESTDIR=$(INSTALLDIR)/libcap
	rm -rf $(INSTALLDIR)/libcap/usr/include
	rm -f $(INSTALLDIR)/libcap/lib/*.a

protobuf-c-configure:
	cd protobuf-c && ./autogen.sh
	cd protobuf-c && ./configure \
				--prefix=/usr \
				--libdir=/usr/lib \
				--target=$(ARCH)-linux \
				--host=$(ARCH) \
				--disable-protoc \
				CFLAGS="-fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libcap/libcap/include -fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT)"  \
				CPPFLAGS="-fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libcap/libcap/include -fPIC -DNEED_PRINTF $(COPTS) $(MIPS16_OPT)"  \
				LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections"

protobuf-c:
	make -C protobuf-c

protobuf-c-clean:
	make -C protobuf-c clean

protobuf-c-install:
	@true

frr-configure: ncurses json-c readline libyang libcap libcares protobuf-c-configure
	make -C protobuf-c
	cd frr && autoreconf --force --install
	cd frr && chmod 777 configure
	rm -rf frr/build
	-mkdir -p frr/build
	cd frr/build && ../configure CC="" CFLAGS="" LDFLAGS="" LD="" --with-vtysh-pager=less --disable-eigrpd --disable-ldpd --enable-shared --disable-pbrd --disable-rfptest --disable-ssd  --disable-doc --enable-clippy-only --enable-shared --disable-zeromq --enable-opaque-lsa --disable-nhrpd --enable-ospf-te --disable-ospfclient --enable-multipath=64  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd --disable-staticd --disable-bfdd --disable-babeld --disable-protobuf --enable-pie=no PYTHON=/usr/bin/python3
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
		--enable-protobuf \
		--disable-ldpd  \
		--disable-rfptest \
		--disable-ssd \
		--enable-shared \
		--disable-static \
		--disable-doc \
		--disable-zeromq \
		--disable-backtrace \
		--enable-opaque-lsa \
		--disable-nhrpd \
		--disable-fabricd \
		--disable-vrrpd \
		--disable-pathd \
		--enable-ospf-te --disable-ospfclient --enable-multipath=64  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d \
		--enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd \
		--disable-staticd --enable-bfdd --disable-babeld --enable-pie=no --with-libreadline=$(TOP)/readline \
		CFLAGS="-fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libcap/libcap/include -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/libcares/include -I$(TOP)/ -Drpl_malloc=malloc -Drpl_realloc=realloc -I$(TOP)/_staging/usr/include -I$(TOP)/frr/build -I$(TOP)/libyang/build -I$(TOP)/libyang/build/src -I$(TOP)/libyang/src/plugins_exts -I$(TOP)/libyang/src -I$(TOP)/protobuf-c" \
		CPPFLAGS="-fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libcap/libcap/include -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) -I$(TOP)/libcares/include -I$(TOP)/ -Drpl_malloc=malloc -Drpl_realloc=realloc -I$(TOP)/_staging/usr/include -I$(TOP)/frr/build -I$(TOP)/libyang/build -I$(TOP)/libyang/build/src -I$(TOP)/libyang/src/plugins_exts -I$(TOP)/libyang/src" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/readline/shlib -L$(TOP)/ncurses/lib -lncurses -L$(TOP)/json-c/.libs -ljson-c -L$(TOP)/libyang/build -lyang -L$(TOP)/pcre2/.libs -lpcre2-8 -L$(TOP)/libcap/libcap -lcap -latomic -L$(TOP)/protobuf-c/protobuf-c/.libs" \
		LIBYANG_CFLAGS="-I$(TOP)/libyang/build -I$(TOP)/libyang/build/src -I$(TOP)/libyang/src/plugins_exts -I$(TOP)/libyang/src -I$(TOP)/pcre2/src -I$(TOP)/libcares/include" \
		LIBYANG_LIBS="-L$(TOP)/libyang/build -lyang -L$(TOP)/pcre2/.libs -lpcre2-8" \
		CARES_CFLAGS="-I$(TOP)/libcares/include" \
		CARES_LIBS="-L$(TOP)/libcares/src/lib/.libs -lcares" \
		PROTOBUF_C_CFLAGS="-I$(TOP)/protobuf-c" \
		PROTOBUF_C_LIBS="-L$(TOP)/protobuf-c/protobuf-c/.libs -lprotobuf-c" \
		PYTHON=/usr/bin/python3

frr: ncurses json-c libyang libcap libcares protobuf-c
	make -C frr/build

frr-install:
	make -C frr/build DESTDIR=$(INSTALLDIR)/frr install
	mkdir -p $(INSTALLDIR)/frr/usr/lib
	-cp -urv $(INSTALLDIR)/frr/usr/tmp/* $(INSTALLDIR)/frr/usr/lib
	rm -rf $(INSTALLDIR)/frr/usr/tmp 

	rm -rf $(INSTALLDIR)/frr/tmp
	rm -rf $(INSTALLDIR)/frr/usr/info
#	rm -rf $(INSTALLDIR)/frr/usr/share
	rm -rf $(INSTALLDIR)/frr/usr/include
	rm -rf $(INSTALLDIR)/frr/usr/etc
	rm -rf $(INSTALLDIR)/frr/usr/man
	rm -f $(INSTALLDIR)/frr/usr/lib/*.a
	rm -f $(INSTALLDIR)/frr/usr/lib/*.la
	rm -f $(INSTALLDIR)/frr/usr/lib/frr/modules/*.la
	rm -f $(INSTALLDIR)/frr/usr/bin/bgp_btoa
	rm -f $(INSTALLDIR)/frr/usr/bin/test_igmpv3_join
	mkdir -p $(INSTALLDIR)/frr/usr/bin

frr-clean:
	-if test -e "frr/build/Makefile"; then $(MAKE) -C frr/build clean; fi
