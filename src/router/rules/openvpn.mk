openvpn-configure: openssl
	cd lzo && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections"
	make -C lzo clean
	make -j 4 -C lzo
	cd openvpn && ./configure --host=$(ARCH)-linux CPPFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --enable-password-save --enable-management --enable-lzo --enable-small --enable-server --enable-multihome CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs  -ffunction-sections -fdata-sections -Wl,--gc-sections"

openvpn: openssl
#ifeq ($(CONFIG_NEWMEDIA),y)
#else
#	cd openvpn && ./configure --host=$(ARCH)-linux CPPFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --disable-management --disable-socks --enable-lzo --enable-small --enable-server --enable-http --enable-password-save CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs  -ffunction-sections -fdata-sections -Wl,--gc-sections"
#endif
	make -j 4 -C lzo clean
	make -j 4 -C lzo
ifneq ($(CONFIG_MADWIFI),y)
	rm -f openssl/*.so*
endif
ifeq ($(CONFIG_NEWMEDIA),y)
	make -j 4 -C openvpn clean
else
	make -j 4 -C openvpn clean
endif
	make -j 4 -C openvpn

openvpn-install:
	install -D openvpn/openvpn $(INSTALLDIR)/openvpn/usr/sbin/openvpn

ifeq ($(CONFIG_AIRNET),y)
	install -D openvpn/config-airnet/openvpncl.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpncl.nvramconfig
	install -D openvpn/config-airnet/openvpncl.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpncl.webvpn
	install -D openvpn/config-airnet/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config-airnet/openvpn.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpn.webvpn
else
	install -D openvpn/config/openvpncl.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpncl.nvramconfig
	install -D openvpn/config/openvpncl.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpncl.webvpn
	install -D openvpn/config2/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config2/openvpn.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpn.webvpn
endif
	cp -f openvpn/config/*.sh $(INSTALLDIR)/openvpn/etc


openvpn-clean:
	if test -e "openvpn/Makefile"; then make -C openvpn clean; fi



