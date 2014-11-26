ifeq ($(CONFIG_NEXTMEDIA),y)
NMAP_EXTRAFLAGS="SENTINEL_FLAGS=-DRAISENTINET3"
endif
nmap-configure:
	cd nmap && ./configure \
		--host=$(ARCH)-linux \
		--prefix=/usr \
		--with-libdnet=included \
		--with-libpcre=included \
		--with-libpcap="../libpcap_noring" \
		--without-liblua \
		CPPFLAGS="-I$(TOP)/libpcap_noring $(COPTS) -DNEED_PRINTF" \
		CFLAGS="-I$(TOP)/libpcap_noring $(COPTS) -DNEED_PRINTF" \
		LDFLAGS="-L$(TOP)/libpcap_noring" PCAP_ROOT="$(TOP)/libpcap_noring"

nmap:
	make -C nmap $(NMAP_EXTRAFLAGS) clean
	make -C nmap $(NMAP_EXTRAFLAGS)

nmap-clean:
	if test -e "nmap/Makefile"; then make -C nmap clean; fi
	@true

nmap-install:
	make -C nmap install DESTDIR=$(INSTALLDIR)/nmap
	rm -rf $(INSTALLDIR)/nmap/usr/share/man
