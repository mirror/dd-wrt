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
SSL_ADDOPT=OPENSSL_LIBS="-L$(SSL_LIB_PATH) -lssl -lcrypto" \
	OPTIONAL_CRYPTO_LIBS="-L$(SSL_LIB_PATH) -lssl -lcrypto" \
	OPENSSL_SSL_CFLAGS="-I$(SSLPATH)/include" \
	OPENSSL_SSL_LIBS="-L$(SSL_LIB_PATH) -lssl" \
	OPENSSL_CRYPTO_CFLAGS="-I$(SSLPATH)/include" \
	OPENSSL_CRYPTO_LIBS="-L$(SSL_LIB_PATH) -lcrypto"
endif



CONFIGURE_ARGS_OVPN += \
	--host=$(ARCH)-linux \
	CPPFLAGS="-I$(TOP)/lzo/include -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs" \
	--prefix=/usr \
	--disable-selinux \
	--disable-systemd \
	--disable-debug \
	--disable-eurephia \
	--disable-pkcs11 \
	--disable-plugins \
	--enable-password-save \
	--enable-management \
	--enable-lzo \
	--enable-fragment \
	--enable-server \
	--enable-multihome \
	--with-crypto-library=$(SSL_TYPE) \
	$(SSL_ADDOPT) \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) -I$(SSLPATH)/include  -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO) -L$(SSL_LIB_PATH) -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -ldl -lpthread -lrt" \
	AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	ac_cv_func_epoll_create=yes \
	ac_cv_path_IFCONFIG=/sbin/ifconfig \
	ac_cv_path_ROUTE=/sbin/route \
	ac_cv_path_IPROUTE=/usr/sbin/ip 

openvpn-conf-prep:
	-rm -f openvpn/Makefile
	cd openvpn && aclocal
	cd openvpn && autoconf
	cd openvpn && automake

openvpn-conf: $(SSL_DEP)
	if ! test -e "$(OVPN)/Makefile"; then cd $(OVPN) && ./configure $(CONFIGURE_ARGS_OVPN); fi 


openvpn-configure: lzo openvpn-conf-prep openvpn-conf

openvpn: lzo $(SSL_DEP) openvpn-conf
#ifeq ($(CONFIG_NEWMEDIA),y)
#else
#	cd $(OVPN) && ./configure --host=$(ARCH)-linux CPPFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -I../lzo/include -I../openssl/include -L../lzo -L../openssl -L../lzo/src/.libs" --enable-static --disable-shared --disable-pthread --disable-plugins --disable-debug --disable-management --disable-socks --enable-lzo --enable-small --enable-server --enable-http --enable-password-save CFLAGS="$(COPTS)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="-L../openssl -L../lzo -L../lzo/src/.libs  -ffunction-sections -fdata-sections -Wl,--gc-sections"
#endif
ifneq ($(CONFIG_FREERADIUS),y)
ifneq ($(CONFIG_ASTERISK),y)
ifneq ($(CONFIG_AIRCRACK),y)
ifneq ($(CONFIG_POUND),y)
ifneq ($(CONFIG_IPETH),y)
ifneq ($(CONFIG_VPNC),y)
ifneq ($(CONFIG_TOR),y)
ifneq ($(CONFIG_ANCHORFREE),y)
ifeq ($(CONFIG_MATRIXSSL),y)
	rm -f openssl/*.so*
endif
endif
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
	-make -C $(OVPN) clean



