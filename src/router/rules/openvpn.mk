WOLFSSL_SSLPATH=$(TOP)/wolfssl
WOLFSSL_SSL_LIB_PATH=$(WOLFSSL_SSLPATH)/standard/src/.libs
WOLFSSL_SSL_DEP=wolfssl
WOLFSSL_SSL_ADDOPT=OPENSSL_LIBS="-L$(WOLFSSL_SSL_LIB_PATH)" \
	WOLFSSL_LIBS="-L$(WOLFSSL_SSL_LIB_PATH) -lwolfssl" \
	WOLFSSL_CFLAGS="-I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl -DWOLFSSL_OPENVPN"


OVPN=openvpn
OPENSSL_SSLPATH=$(TOP)/openssl
OPENSSL_SSL_LIB_PATH=$(OPENSSL_SSLPATH)
OPENSSL_SSL_DEP=openssl
OPENSSL_SSL_ADDOPT=OPENSSL_LIBS="-L$(OPENSSL_SSL_LIB_PATH) -lssl -lcrypto -lpthread" \
	OPTIONAL_CRYPTO_LIBS="-L$(OPENSSL_SSL_LIB_PATH) -lssl -lcrypto -lpthread" \
	OPENSSL_SSL_CFLAGS="-I$(OPENSSL_SSLPATH)/include" \
	OPENSSL_SSL_LIBS="-L$(OPENSSL_SSL_LIB_PATH) -lssl" \
	OPENSSL_CRYPTO_CFLAGS="-I$(OPENSSL_SSLPATH)/include" \
	OPENSSL_CRYPTO_LIBS="-L$(OPENSSL_SSL_LIB_PATH) -lcrypto -lpthread"

DCO=--disable-dco
ifeq ($(KERNELVERSION),6.1)
DCO=--enable-dco
OVPN_LIBNL_CFLAGS=-I$(TOP)/libnl/include/linux-private -I$(TOP)/libnl/include
OVPN_LIBNL_LIBS=-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3
endif
ifeq ($(KERNELVERSION),6.6)
DCO=--enable-dco
OVPN_LIBNL_CFLAGS=-I$(TOP)/libnl/include/linux-private -I$(TOP)/libnl/include
OVPN_LIBNL_LIBS=-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3
endif
ifeq ($(KERNELVERSION),4.9)
DCO=--enable-dco
OVPN_LIBNL_CFLAGS=-I$(TOP)/libnl/include/linux-private -I$(TOP)/libnl/include
OVPN_LIBNL_LIBS=-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3
endif
ifeq ($(KERNELVERSION),4.14)
DCO=--enable-dco
OVPN_LIBNL_CFLAGS=-I$(TOP)/libnl/include/linux-private -I$(TOP)/libnl/include
OVPN_LIBNL_LIBS=-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3
endif
ifeq ($(KERNELVERSION),4.4)
DCO=--enable-dco
OVPN_LIBNL_CFLAGS=-I$(TOP)/libnl/include/linux-private -I$(TOP)/libnl/include
OVPN_LIBNL_LIBS=-L$(TOP)/libnl/lib/.libs -lnl-3 -lnl-genl-3
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
	--disable-lz4 \
	--enable-fragment \
	--enable-server \
	--enable-multihome \
	$(DCO) \
	--with-crypto-library=openssl \
	$(OPENSSL_SSL_ADDOPT) \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(LTOFIXUP) -I$(OPENSSL_SSLPATH)/include  -DNEED_PRINTF $(OVPN_LIBNL_CFLAGS) -std=c99 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO) $(LTOFIXUP) -L$(OPENSSL_SSL_LIB_PATH) -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -ldl -lpthread -L$(TOP)/libucontext -lucontext $(OVPN_LIBNL_LIBS)" \
	LZO_CFLAGS="-I$(TOP)/lzo/include" \
	LZO_LIBS="-L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -llzo2" \
	AR_FLAGS="\"cru $(LTOPLUGIN)\"" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	ac_cv_func_epoll_create=yes \
	ac_cv_path_IFCONFIG=/sbin/ifconfig \
	ac_cv_path_ROUTE=/sbin/route \
	ac_cv_path_IPROUTE=/usr/sbin/ip

CONFIGURE_ARGS_WOLFSSL += \
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
	--disable-lz4 \
	--enable-fragment \
	--enable-server \
	--enable-multihome \
	$(DCO) \
	--with-crypto-library=wolfssl \
	$(WOLFSSL_SSL_ADDOPT) \
	CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(LTOFIXUP) -I$(WOLFSSL_SSLPATH)/include $(OVPN_LIBNL_CFLAGS)  -DNEED_PRINTF  -std=c99 -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections  $(LDLTO) $(LTOFIXUP) -L$(WOLFSSL_SSL_LIB_PATH) $(OVPN_LIBNL_LIBS) -L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -ldl -lpthread" \
	LZO_CFLAGS="-I$(TOP)/lzo/include" \
	LZO_LIBS="-L$(TOP)/lzo -L$(TOP)/lzo/src/.libs -llzo2" \
	AR_FLAGS="\"cru $(LTOPLUGIN)\"" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	ac_cv_func_epoll_create=yes \
	ac_cv_path_IFCONFIG=/sbin/ifconfig \
	ac_cv_path_ROUTE=/sbin/route \
	ac_cv_path_IPROUTE=/usr/sbin/ip


ifeq ($(ARCHITECTURE),broadcom)
ifneq ($(CONFIG_BCMMODERN),y)
CONFIGURE_ARGS_OVPN += ac_cv_func_epoll_create=no --enable-iproute2
CONFIGURE_ARGS_WOLFSSL += ac_cv_func_epoll_create=no  --enable-iproute2
else
CONFIGURE_ARGS_OVPN += ac_cv_func_epoll_create=yes
CONFIGURE_ARGS_WOLFSSL += ac_cv_func_epoll_create=yes
endif
else
CONFIGURE_ARGS_OVPN += ac_cv_func_epoll_create=yes
CONFIGURE_ARGS_WOLFSSL += ac_cv_func_epoll_create=yes
endif

openvpn-conf-prep:
	-rm -f openvpn/Makefile
	cd openvpn && libtoolize
	cd openvpn && aclocal
	cd openvpn && autoconf
	cd openvpn && autoheader
	cd openvpn && automake --add-missing

openvpn-conf: openssl wolfssl
	mkdir -p openvpn/openssl
	mkdir -p openvpn/wolfssl
	-$(MAKE) -C wolfssl/minimal
	-$(MAKE) -C wolfssl/standard
	-cd $(OVPN)/openssl && ../configure $(CONFIGURE_ARGS_OVPN)
	-cd $(OVPN)/wolfssl && ../configure $(CONFIGURE_ARGS_WOLFSSL)


openvpn-configure: lzo openvpn-conf-prep openvpn-conf libnl

openvpn: lzo $(SSL_DEP) libnl
ifeq ($(CONFIG_AIRNET),y)
	install -D openvpn/config-airnet/openvpncl.webvpn $(TOP)/httpd/ej_temp/openvpncl.webvpn
	install -D openvpn/config-airnet/openvpn.webvpn $(TOP)/httpd/ej_temp/openvpn.webvpn
else
	install -D openvpn/config/openvpncl.webvpn $(TOP)/httpd/ej_temp/openvpncl.webvpn
	install -D openvpn/config/openvpn.webvpn $(TOP)/httpd/ej_temp/openvpn.webvpn
endif
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
ifneq ($(CONFIG_DDNS),y)
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
endif
	-make -C $(OVPN)/openssl clean
ifeq ($(CONFIG_OPENVPN_SSLSTATIC),y)
	rm -f openssl/*.so*
endif
ifeq ($(CONFIG_WOLFSSL),y)
	make -C $(OVPN)/wolfssl
else
	make -C $(OVPN)/openssl
endif
ifeq ($(KERNELVERSION),6.1)
	make -C ovpn-dco
endif
ifeq ($(KERNELVERSION),6.6)
	make -C ovpn-dco
endif
ifeq ($(KERNELVERSION),4.9)
	make -C ovpn-dco
endif
ifeq ($(KERNELVERSION),4.14)
	make -C ovpn-dco
endif
ifeq ($(KERNELVERSION),4.4)
	make -C ovpn-dco
endif

openvpn-install:
ifeq ($(CONFIG_WOLFSSL),y)
	make -C $(OVPN)/wolfssl install DESTDIR=$(INSTALLDIR)/openvpn
else
	make -C $(OVPN)/openssl install DESTDIR=$(INSTALLDIR)/openvpn
endif
	rm -rf $(INSTALLDIR)/openvpn/usr/share
	rm -rf $(INSTALLDIR)/openvpn/usr/include
ifeq ($(CONFIG_AIRNET),y)
	install -D openvpn/config-airnet/openvpncl.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpncl.nvramconfig
	install -D openvpn/config-airnet/openvpncl.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpncl.webvpn
	install -D openvpn/config-airnet/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config-airnet/openvpn.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpn.webvpn
else
	install -D openvpn/config/openvpncl.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpncl.nvramconfig
	install -D openvpn/config/openvpncl.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpncl.webvpn
	install -D openvpn/config/openvpn.nvramconfig $(INSTALLDIR)/openvpn/etc/config/openvpn.nvramconfig
	install -D openvpn/config/openvpn.webvpn $(INSTALLDIR)/openvpn/etc/config/openvpn.webvpn
endif
	cp -f openvpn/config/*.sh $(INSTALLDIR)/openvpn/etc
	install -D -m 0755 openvpn/config/userscripts/*.sh -t $(INSTALLDIR)/openvpn/usr/bin
ifeq ($(KERNELVERSION),6.1)
	make -C ovpn-dco install
endif
ifeq ($(KERNELVERSION),6.6)
	make -C ovpn-dco install
endif
ifeq ($(KERNELVERSION),4.9)
	make -C ovpn-dco install
endif
ifeq ($(KERNELVERSION),4.14)
	make -C ovpn-dco install
endif
ifeq ($(KERNELVERSION),4.4)
	make -C ovpn-dco install
endif

openvpn-clean:
	-make -C $(OVPN)/wolfssl clean
	-make -C $(OVPN)/openssl clean
ifeq ($(KERNELVERSION),6.1)
	make -C ovpn-dco clean
endif
ifeq ($(KERNELVERSION),6.6)
	make -C ovpn-dco clean
endif
ifeq ($(KERNELVERSION),4.9)
	make -C ovpn-dco clean
endif
ifeq ($(KERNELVERSION),4.14)
	make -C ovpn-dco clean
endif
ifeq ($(KERNELVERSION),4.4)
	make -C ovpn-dco clean
endif
