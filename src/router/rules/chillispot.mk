chillispot-configure:
	cd chillispot && ./configure --host=$(ARCH)-linux-elf CFLAGS="$(COPTS) -DHAVE_MALLOC=1 -Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections"

chillispot:
	$(MAKE) -j 4 -C chillispot

chillispot-install:
ifneq ($(CONFIG_FON),y)
	install -D chillispot/config/chillispot.nvramconfig $(INSTALLDIR)/chillispot/etc/config/chillispot.nvramconfig
	install -D chillispot/config/chillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispot.webhotspot
endif
ifeq ($(CONFIG_TIEXTRA1),y)
	install -D private/telkom/mchillispot.webhotspot $(INSTALLDIR)/chillispot/etc/config/chillispotm.webhotspot
endif
ifeq ($(CONFIG_CHILLILOCAL),y)
	install -D chillispot/config/fon.nvramconfig $(INSTALLDIR)/chillispot/etc/config/fon.nvramconfig
	install -D chillispot/config/fon.webhotspot $(INSTALLDIR)/chillispot/etc/config/fon.webhotspot
endif
ifeq ($(CONFIG_HOTSPOT),y)
	install -D chillispot/config/hotss.nvramconfig $(INSTALLDIR)/chillispot/etc/config/hotss.nvramconfig
	install -D chillispot/config/3hotss.webhotspot $(INSTALLDIR)/chillispot/etc/config/3hotss.webhotspot
endif
	install -D chillispot/src/chilli $(INSTALLDIR)/chillispot/usr/sbin/chilli
	$(STRIP) $(INSTALLDIR)/chillispot/usr/sbin/chilli

chillispot-clean:
	$(MAKE) -C chillispot clean
