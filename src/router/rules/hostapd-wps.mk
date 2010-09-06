ifeq ($(CONFIG_ATH9K),y)
ATH9K_CFLAGS=-I$(TOP)/libnl-tiny/include \
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE
ATH9K_LDFLAGS=-L$(TOP)/libnl-tiny/ -lm -lnl-tiny
endif

hostapd2: 
	$(MAKE) -C hostapd-wps/hostapd clean
	$(MAKE) -C hostapd-wps/wpa_supplicant clean
	echo ` \
		$(MAKE) -s -C hostapd-wps/hostapd MULTICALL=1 dump_cflags; \
		$(MAKE) -s -C hostapd-wps/wpa_supplicant MULTICALL=1 dump_cflags \
	` > hostapd-wps/.cflags

	

	$(MAKE) -C hostapd-wps/hostapd CFLAGS="$$(cat hostapd-wps/.cflags) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) CONFIG_DD_DIST=$(CONFIG_DIST) MULTICALL=1 hostapd_cli hostapd_multi.a
	$(MAKE) -C hostapd-wps/wpa_supplicant CFLAGS="$$(cat hostapd-wps/.cflags) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) CONFIG_DD_DIST=$(CONFIG_DIST) MULTICALL=1 wpa_cli wpa_supplicant_multi.a
	$(CC) $(COPTS) -L$(TOP)/nvram  -L$(TOP)/libutils -Wall -ffunction-sections -fdata-sections -Wl,--gc-sections -o hostapd-wps/wpad hostapd-wps/multicall/multicall.c \
		hostapd-wps/hostapd/hostapd_multi.a \
		hostapd-wps/wpa_supplicant/wpa_supplicant_multi.a \
		$(ATH9K_LDFLAGS) -lutils -lnvram

hostapd2-clean:
	$(MAKE) -C hostapd-wps/hostapd clean
	$(MAKE) -C hostapd-wps/wpa_supplicant clean

hostapd2-install:
	install -D hostapd-wps/wpad $(INSTALLDIR)/hostapd2/usr/sbin/wpad
ifeq ($(CONFIG_WPS),y)
	install -D hostapd-wps/hostapd/hostapd_cli $(INSTALLDIR)/hostapd2/usr/sbin/hostapd_cli
endif
	cd $(INSTALLDIR)/hostapd2/usr/sbin && ln -sf wpad hostapd
	cd $(INSTALLDIR)/hostapd2/usr/sbin && ln -sf wpad wpa_supplicant

