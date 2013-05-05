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
	--disable-debug \
	--enable-binstatusfile 
ifeq ($(CONFIG_COOVA_CHILLI),y)
CHILLIDIR=$(CHILLICOOVADIR)
CHILLIEXTRAFLAGS=$(CHILLICOOVAEXTRAFLAGS)
else
CHILLIDIR=chillispot
endif

chillispot-configure:
	cd $(CHILLIDIR) && ./configure $(CHILLIEXTRAFLAGS) --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections"
	cd $(CHILLICOOVADIR) && ./configure $(CHILLICOOVAEXTRAFLAGS) --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections"

chillispot:
	$(MAKE) -j 4 -C $(CHILLIDIR)

chillispot-install:
ifneq ($(CONFIG_FON),y)
	install -D $(CHILLIDIR)/config/chillispot.nvramconfig $(INSTALLDIR)/chillispot/etc/config/chillispot.nvramconfig
	install -D $(CHILLIDIR)/config/chillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispot.webhotspot
endif
ifeq ($(CONFIG_TIEXTRA1),y)
	install -D private/telkom/mchillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispotm.webhotspot
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
	install -D $(CHILLIDIR)/config/fon.nvramconfig $(INSTALLDIR)/chillispot/etc/config/fon.nvramconfig
	install -D $(CHILLIDIR)/config/fon.webhotspot $(INSTALLDIR)/chillispot/etc/config/fon.webhotspot
endif
ifeq ($(CONFIG_HOTSPOT),y)
	install -D $(CHILLIDIR)/config/hotss.nvramconfig $(INSTALLDIR)/chillispot/etc/config/hotss.nvramconfig
	install -D $(CHILLIDIR)/config/3hotss.webhotspot $(INSTALLDIR)/chillispot/etc/config/3hotss.webhotspot
endif
	install -D $(CHILLIDIR)/src/chilli $(INSTALLDIR)/chillispot/usr/sbin/chilli
ifeq ($(CONFIG_COOVA_CHILLI),y)
	-install -D $(CHILLIDIR)/src/chilli_opt $(INSTALLDIR)/chillispot/usr/sbin/chilli_opt
	-install -D $(CHILLIDIR)/src/chilli_query $(INSTALLDIR)/chillispot/usr/sbin/chilli_query
	-install -D $(CHILLIDIR)/src/chilli_radconfig $(INSTALLDIR)/chillispot/usr/sbin/chilli_radconfig
	-install -D $(CHILLIDIR)/src/chilli_response $(INSTALLDIR)/chillispot/usr/sbin/chilli_response
endif
	$(STRIP) $(INSTALLDIR)/chillispot/usr/sbin/chilli*

chillispot-clean:
	$(MAKE) -C $(CHILLIDIR) clean
