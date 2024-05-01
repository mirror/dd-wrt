quagga-configure: ncurses readline
	cd quagga && autoreconf --force --install
	cd quagga && ./configure --host=$(ARCH)-uclibc-linux $(CONFIG_QUAGGA_EXTRA) --localstatedir=/var/run  --libdir=/usr/tmp --enable-opaque-lsa --disable-nhrpd --enable-ospf-te --disable-ospfclient --enable-multipath=32  --enable-ipv6 --prefix=/usr --sysconfdir=/tmp --disable-ospf6d  --enable-vtysh --enable-user=root --enable-group=root --disable-ospfapi --disable-isisd --disable-pimd --disable-nhrpd --enable-pie=no --with-libreadline=$(TOP)/readline CFLAGS="-fno-strict-aliasing -I$(TOP)/ -DNEED_PRINTF -fcommon -Drpl_malloc=malloc -Drpl_realloc=realloc $(COPTS)  $(MIPS16_OPT) $(THUMB)" LDFLAGS="-L$(TOP)/readline/shlib -L$(TOP)/ncurses/lib -lncurses" 
	cd quagga && touch *

quagga: ncurses
	$(MAKE) -C quagga

quagga-install:
	make -C quagga DESTDIR=$(INSTALLDIR)/quagga install
	mkdir -p $(INSTALLDIR)/quagga/usr/lib
	-cp -urv $(INSTALLDIR)/quagga/usr/tmp/* $(INSTALLDIR)/quagga/usr/lib
	rm -rf $(INSTALLDIR)/quagga/usr/tmp 	
ifneq ($(CONFIG_MUSL),y)
	rm -f $(INSTALLDIR)/quagga/usr/bin/vtysh*
endif
	rm -rf $(INSTALLDIR)/quagga/tmp
	rm -rf $(INSTALLDIR)/quagga/usr/info
	rm -rf $(INSTALLDIR)/quagga/usr/share
	rm -rf $(INSTALLDIR)/quagga/usr/include
	rm -rf $(INSTALLDIR)/quagga/usr/etc
	rm -rf $(INSTALLDIR)/quagga/usr/man
	rm -f $(INSTALLDIR)/quagga/usr/lib/*.a
	rm -f $(INSTALLDIR)/quagga/usr/lib/*.la
	rm -f $(INSTALLDIR)/quagga/usr/bin/bgp_btoa
	rm -f $(INSTALLDIR)/quagga/usr/bin/test_igmpv3_join
	mkdir -p $(INSTALLDIR)/quagga/usr/bin
	printf "#!/bin/sh\n" > $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram set wk_mode=\"ospf bgp rip router\"\n" >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram set zebra_copt=1\n" >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram set ospfd_copt=1\n" >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram set ripd_copt=1\n"  >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram set bgpd_copt=1\n"  >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "nvram commit\n"           >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "stopservice zebra\n"      >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	printf "startservice zebra\n"     >> $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh
	chmod 755 $(INSTALLDIR)/quagga/usr/bin/vtysh_init.sh

quagga-clean:
	-if test -e "quagga/Makefile"; then $(MAKE) -C quagga clean; fi
