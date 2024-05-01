export ac_cv_lib_rt_clock_gettime=no
CHILLICOOVADIR=coova-chilli
CHILLICOOVAEXTRAFLAGS=--enable-uamdomainfile \
	--prefix=/usr \
	--datadir=/usr/share \
	--localstatedir=/var \
	--sysconfdir=/etc \
	--disable-miniportal \
	--sbindir=/usr/sbin \
	--enable-shared \
	--disable-static \
	--enable-miniconfig \
	--disable-debug \
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
	cd $(CHILLIDIR) && ./bootstrap
	cd $(CHILLIDIR) &&  rm -rf config.{cache,status} && ./configure $(CHILLIEXTRAFLAGS) --host=$(ARCH)-linux  --disable-shared --enable-static \
	    CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(THUMB) $(CHILLIEXTRA_CFLAGS) -fcommon -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	    LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
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
	rm -f coova-chilli/src/cmdline.c
	rm -f coova-chilli/src/cmdline.h
	$(MAKE) -C $(CHILLIDIR)

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
	install -D $(CHILLIDIR)/src/chilli_multicall $(INSTALLDIR)/chillispot/usr/sbin/chilli_multicall
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_opt
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_query
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_radconfig
	cd $(INSTALLDIR)/chillispot/usr/sbin && ln -sf chilli_multicall chilli_response

chillispot-clean:
	$(MAKE) -C $(CHILLIDIR) clean
