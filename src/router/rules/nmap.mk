ifeq ($(CONFIG_NEXTMEDIA),y)
NMAP_EXTRAFLAGS="SENTINEL_FLAGS=-DRAISENTINET3"
endif
nmap-configure:
	cd nmap && ./configure \
		--host=$(ARCH)-linux \
		--prefix=/usr \
		--with-libdnet=included \
		--with-libpcre=included \
		--with-libpcap="../libpcap" \
		--with-liblua=included \
		--with-libssh2=included \
		--with-openssl=$(TOP)/openssl \
		--with-libz=$(TOP)/zlib \
		--without-zenmap \
		--without-ncat \
		--without-nmap-update \
		--without-ndiff \
		--without-nping \
		CPPFLAGS="-I$(TOP)/libpcap -I$(TOP)/openssl/include -L$(TOP)/openssl $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-I$(TOP)/libpcap -I$(TOP)/openssl/include -L$(TOP)/openssl $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CXXFLAGS="-I$(TOP)/libpcap -I$(TOP)/openssl/include $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDFLAGS="-L$(TOP)/libpcap -L$(TOP)/openssl $(COPTS) -lssl -lcrypto" PCAP_ROOT="$(TOP)/libpcap  -ffunction-sections -fdata-sections -Wl,--gc-sections"

nmap:
	cd nmap && find . -name makefile.dep -delete	
	make -C nmap $(NMAP_EXTRAFLAGS) clean
	make -C nmap $(NMAP_EXTRAFLAGS)

nmap-clean:
	if test -e "nmap/Makefile"; then make -C nmap clean; fi
	@true

nmap-install:
	make -C nmap install DESTDIR=$(INSTALLDIR)/nmap
	rm -rf $(INSTALLDIR)/nmap/usr/share/man
	rm -rf $(INSTALLDIR)/nmap/usr/share/zenmap/docs
	rm -rf $(INSTALLDIR)/nmap/usr/share/applications
