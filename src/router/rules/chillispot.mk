export ac_cv_lib_rt_clock_gettime=no
CHILLICOOVADIR=coova-chilli
CHILLICOOVAEXTRAFLAGS=--enable-uamdomainfile \
	--prefix=/usr \
	--datadir=/usr/share \
	--localstatedir=/var \
	--sysconfdir=/etc \
	--enable-miniportal \
	--enable-chilliproxy \
	--enable-chilliredir \
	--enable-useragent \
	--sbindir=/usr/sbin \
	--enable-shared \
	--disable-static \
	--enable-miniconfig \
	--disable-debug \
	--with-nfcoova \
	--disable-binstatusfile 
ifeq ($(ARCHITECTURE),broadcom)
ifneq ($(CONFIG_BCMMODERN),y)
CHILLICOOVAEXTRAFLAGS+=--without-ipv6
endif
endif
ifeq ($(ARCHITECTURE),storm)
CHILLICOOVAEXTRAFLAGS+=--without-ipv6
endif
ifeq ($(ARCHITECTURE),openrisc)
CHILLICOOVAEXTRAFLAGS+=--without-ipv6
endif
ifeq ($(ARCHITECTURE),adm5120)
CHILLICOOVAEXTRAFLAGS+=--without-ipv6
endif
CHILLIEXTRA_CFLAGS = $(MIPS16_OPT) $(THUMB) 
CHILLIDIR=$(CHILLICOOVADIR)
CHILLIEXTRAFLAGS=$(CHILLICOOVAEXTRAFLAGS)

chillispot-configure:
	rm -rf /tmp/nossl
	rm -rf /tmp/ssl
	rm -rf $(CHILLIDIR)/nossl
	rm -rf $(CHILLIDIR)/openssl
	mkdir -p $(CHILLIDIR)/nossl
	mkdir -p $(CHILLIDIR)/openssl
	cp -urv $(CHILLIDIR) /tmp/ssl
	cp -urv $(CHILLIDIR) /tmp/nossl
	cp -urv /tmp/ssl/* $(CHILLIDIR)/openssl
	cp -urv /tmp/nossl/* $(CHILLIDIR)/nossl
	cd $(CHILLIDIR)/nossl && ./bootstrap
	cd $(CHILLIDIR)/openssl && ./bootstrap
	cd $(CHILLIDIR)/nossl &&  rm -rf config.{cache,status} && ./configure $(CHILLIEXTRAFLAGS) --host=$(ARCH)-linux  --disable-shared --enable-static --without-openssl \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(CHILLIEXTRA_CFLAGS) -I$(TOP)/$(CHILLIDIR)/bstring -I$(TOP)/$(CHILLIDIR) -fcommon -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

	-cd $(CHILLIDIR)/openssl &&  rm -rf config.{cache,status} && ./configure $(CHILLIEXTRAFLAGS) --host=$(ARCH)-linux  --disable-shared --enable-static --with-openssl \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(CHILLIEXTRA_CFLAGS) -I$(SSLPATH)/include  -I$(TOP)/$(CHILLIDIR)/bstring -I$(TOP)/$(CHILLIDIR) -fcommon -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -L$(SSLPATH) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    AR_FLAGS="cru $(LTOPLUGIN)" \
	    RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

chillispot:
ifneq ($(CONFIG_FON),y)
	install -D $(CHILLIDIR)/config/chillispot.webhotspot httpd/ej_temp/chillispot.webhotspot
endif
ifeq ($(CONFIG_TIEXTRA1),y)
	install -D private/telkom/mchillispot.webhotspot httpd/ej_temp/chillispotm.webhotspot
endif
#ifeq ($(CONFIG_CHILLILOCAL),y)
#	install -D $(CHILLIDIR)/config/fon.webhotspot httpd/ej_temp/fon.webhotspot
#endif
ifeq ($(CONFIG_HOTSPOT),y)
	install -D $(CHILLIDIR)/config/3hotss.webhotspot httpd/ej_temp/3hotss.webhotspot
endif
#	rm -f coova-chilli/nossl/src/cmdline.c
#	rm -f coova-chilli/openssl/src/cmdline.h
	-make -j 1 -C $(CHILLIDIR)/nossl
	-make -j 1 -C $(CHILLIDIR)/openssl
	-make -j 1 -C $(CHILLIDIR)/nossl
	-make -j 1 -C $(CHILLIDIR)/openssl

chillispot-install:
ifneq ($(CONFIG_FON),y)
	install -D $(CHILLIDIR)/config/chillispot.nvramconfig $(INSTALLDIR)/chillispot/etc/config/chillispot.nvramconfig
	install -D $(CHILLIDIR)/config/chillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispot.webhotspot
endif
ifeq ($(CONFIG_TIEXTRA1),y)
	install -D private/telkom/mchillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispotm.webhotspot
endif
ifeq ($(CONFIG_HOTSPOT),y)
	install -D $(CHILLIDIR)/config/hotss.nvramconfig $(INSTALLDIR)/chillispot/etc/config/hotss.nvramconfig
	install -D $(CHILLIDIR)/config/3hotss.webhotspot $(INSTALLDIR)/chillispot/etc/config/3hotss.webhotspot
endif
	mkdir -p $(INSTALLDIR)/chillispot/lib/modules/$(KERNEL_VERSION)/
ifeq ($(CONFIG_OPENSSL),y)
	install -D $(CHILLIDIR)/openssl/src/chilli_multicall $(INSTALLDIR)/chillispot/usr/sbin/chilli_multicall
	-install -D $(CHILLIDIR)/openssl/src/linux/xt_coova.ko $(INSTALLDIR)/chillispot/lib/modules/$(KERNEL_VERSION)/
else
	install -D $(CHILLIDIR)/nossl/src/chilli_multicall $(INSTALLDIR)/chillispot/usr/sbin/chilli_multicall
	-install -D $(CHILLIDIR)/nossl/src/linux/xt_coova.ko $(INSTALLDIR)/chillispot/lib/modules/$(KERNEL_VERSION)/
endif
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_opt
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_query
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_radconfig
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_response
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_proxy
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_radsec
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_redir
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_rtmon
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_script

chillispot-clean:
	$(MAKE) -C $(CHILLIDIR)/nossl clean
	-$(MAKE) -C $(CHILLIDIR)/openssl clean
