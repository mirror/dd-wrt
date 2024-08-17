ifeq ($(ARCH),armeb)
	KERNEL_ARCH := arm
else
ifeq ($(ARCH),mipsel)
	KERNEL_ARCH := mips
else
ifeq ($(ARCH),mips64)
	KERNEL_ARCH := mips
else
	KERNEL_ARCH := $(ARCH)
endif
endif
endif

PKG_MOD_AVAILABLE:= \
	aes \
	af-alg \
	agent \
	attr \
	attr-sql \
	blowfish \
	constraints \
	coupling \
	des \
	dhcp \
	dnskey \
	duplicheck \
	eap-md5 \
	eap-mschapv2 \
	eap-identity \
	eap-aka \
	eap-radius \
	farp \
	test-vectors \
	fips-prf \
	gmp \
	hmac \
	kernel-klips \
	kernel-netlink \
	kernel-pfkey \
	led \
	load-tester \
	md5 \
	pem \
	pgp \
	pkcs1 \
	pubkey \
	random \
	resolve \
	revocation \
	sha1 \
	sha2 \
	socket-default \
	socket-raw \
	sql \
	stroke \
	updown \
	whitelist \
	x509 \
	xauth \
	xauth_eap \
	xcbc \
	sqlite

#	ldap 
#	uci 
#	curl smp sqlite mysql gcrypt medsrv medcli padlock 

CONFIGURE_ARGS=\
	--enable-cisco-quirks \
	--enable-nat-transport \
	--enable-vendor-id \
	--enable-xauth-vid \
	--disable-scripts \
	--disable-static \
	--disable-fast \
	--disable-gcrypt \
	--enable-openssl \
	--enable-tools \
	--prefix=/usr \
	--sysconfdir=/tmp/ipsecetc \
	--localstatedir=/var \
	--libexecdir=/usr/lib \
	--with-ipseclibdir=/usr/lib \
	--with-plugindir=/usr/lib/ipsec/plugins \
	--with-random-device=/dev/random \
	--with-urandom-device=/dev/urandom \
	--with-routing-table=220 \
	--with-routing-table-prio=220 \
	$(foreach m,$(PKG_MOD_AVAILABLE), \
	  $(if $(CONFIG_PACKAGE_strongswan4-mod-$(m)),--enable-$(m),--enable-$(m)) \
	) \

#	--with-linux-headers=$(LINUXDIR)/include 

strongswan-configure: gmp sqlite
	export CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/gmp -I$(SSLPATH)/include -I$(TOP)/sqlite" \
	export CPPFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/gmp -I$(SSLPATH)/include -I$(TOP)/sqlite" ;\
	export LDFLAGS="-L$(TOP)/gmp/.libs -L$(SSLPATH) -L$(TOP)/sqlite/.libs" ; \
	cd strongswan && ./configure --host=$(ARCH)-linux $(CONFIGURE_ARGS)

strongswan: gmp sqlite
	#$(MAKE) -C strongswan CFLAGS="$(COPTS) -DNEED_PRINTF -include `pwd`/strongswan/config.h -I$(TOP)/gmp -I$(SSLPATH)/include" LDFLAGS="-L$(TOP)/gmp/.libs -L$(SSLPATH)"
	$(MAKE) -C strongswan CFLAGS="-DNEED_PRINTF -include `pwd`/strongswan/config.h"

#-I$(LINUXDIR)/include
strongswan-install:
	$(MAKE) -C strongswan install DESTDIR=$(INSTALLDIR)/strongswan
	rm -rf $(INSTALLDIR)/strongswan/usr/share
	-rm -f $(INSTALLDIR)/strongswan/usr/lib/*.la
	-rm -f $(INSTALLDIR)/strongswan/usr/lib/ipsec/plugins/*.la
	-$(STRIP) $(INSTALLDIR)/strongswan/usr/lib/*
	-$(STRIP) $(INSTALLDIR)/strongswan/usr/lib/ipsec/plugins/*

