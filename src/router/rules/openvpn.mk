ifeq ($(CONFIG_POLARSSL),y)
OVPN=openvpn-polarssl
SSLPATH=$(TOP)/polarssl
SSL_LIB_PATH=$(SSLPATH)/library
SSL_TYPE=polarssl
SSL_DEP=polarssl
SSL_ADDOPT=--with-pkcs11-helper-headers=$(TOP)/pkcs11-helper/include \
	   --with-pkcs11-helper-lib=$(TOP)/pkcs11-helper/lib/.libs \
           POLARSSL_CFLAGS="-I$(SSLPATH)/include"  \
           POLARSSL_LIBS="-L$(SSL_LIB_PATH) -lpolarssl"
else
OVPN=openvpn
SSLPATH=$(TOP)/openssl
SSL_LIB_PATH=$(SSLPATH)
SSL_TYPE=openssl
SSL_DEP=openssl
SSL_ADDOPT=OPENSSL_CRYPTO_CFLAGS="-I$(SSLPATH)/include" \
	OPENSSL_SSL_CFLAGS="-I$(SSLPATH)/include" \
	OPENSSL_CRYPTO_LIBS="-L$(SSL_LIB_PATH) -lcrypto" \
	OPENSSL_SSL_LIBS="-L$(SSL_LIB_PATH) -lssl"
endif



CONFIGURE_ARGS_OVPN += \
	--host=$(ARCH)-linux \
	CPPFLAGS="-I$(TOP)/lzo/include -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs" \
	--disable-plugins \
	--enable-debug \
	--enable-password-save \
	--enable-management \
	--enable-lzo \
	--enable-server \
	--enable-multihome \
	--with-crypto-library=$(SSL_TYPE) \
	$(SSL_ADDOPT) \
	CFLAGS="$(COPTS) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -ldl" \
	ac_cv_func_epoll_create=yes

openvpn-conf-prep:
	-rm -f openvpn/Makefile

openvpn-conf: $(SSL_DEP)
	if ! test -e "lzo/Makefile"; then cd lzo && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS)"; fi
	make -j 4 -C lzo
	#if ! test -e "$(OVPN)/Makefile"; then cd $(OVPN) && ./configure --host=$(ARCH)-linux CPPFLAGS="-I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-pthread --disable-plugins --enable-debug --enable-password-save --enable-management --enable-lzo --enable-server --enable-multihome CFLAGS="$(COPTS)" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs -ldl" ac_cv_func_epoll_create=no; fi 
	if ! test -e "$(OVPN)/Makefile"; then cd $(OVPN) && ./configure $(CONFIGURE_ARGS_OVPN); fi 


openvpn-configure: openvpn-conf-prep openvpn-conf

openvpn: $(SSL_DEP) openvpn-conf
#ifeq ($(CONFIG_NEWMEDIA),y)
#else
#	cd $(OVPN) && ./configure --host=$(ARCH)-linux CPPFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --disable-management --disable-socks --enable-lzo --enable-small --enable-server --enable-http --enable-password-save CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs  -ffunction-sections -fdata-sections -Wl,--gc-sections"
#endif
	make -j 4 -C lzo clean
	make -j 4 -C lzo
ifneq ($(CONFIG_FREERADIUS),y)
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_AIRCRACK),y)
ifneq ($(CONFIG_POUND),y)
ifneq ($(CONFIG_IPETH),y)
ifneq ($(CONFIG_VPNC),y)
ifneq ($(CONFIG_TOR),y)
	rm -f openssl/*.so*
endif
endif
endif
endif
endif
endif
endif
ifeq ($(CONFIG_NEWMEDIA),y)
	make -j 4 -C $(OVPN) clean
else
	make -j 4 -C $(OVPN) clean
endif
ifeq ($(CONFIG_OPENVPN_SSLSTATIC),y)
	rm -f openssl/*.so*
endif
	make -j 4 -C $(OVPN)

openvpn-install:
	install -D $(OVPN)/src/openvpn/openvpn $(INSTALLDIR)/openvpn/usr/sbin/openvpn

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
	if test -e "$(OVPN)/Makefile"; then make -C $(OVPN) clean; fi



