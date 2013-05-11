ifeq ($(CONFIG_ATH9K),y)
ATH9K_CFLAGS=-I$(TOP)/libnl-tiny/include\
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE
ATH9K_LDFLAGS=-L$(TOP)/libnl-tiny/ -lm -lnl-tiny
endif
ifeq ($(CONFIG_TIEXTRA1),y)
ATH9K_LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
endif
ifeq ($(CONFIG_TIEXTRA2),y)
ATH9K_LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
endif
ifeq ($(CONFIG_SAMBA3),y)
ATH9K_LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
endif
ifeq ($(CONFIG_FTP),y)
ATH9K_LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
endif
ifeq ($(CONFIG_MINIDLNA),y)
ATH9K_LDFLAGS += -Wl,-rpath,$(TOP)/jansson/src/.libs
endif

ifndef $(HOSTAPDVERSION)
HOSTAPDVERSION=20120910
#HOSTAPDVERSION=wps
endif

ATH9K_CFLAGS += $(MIPS16_OPT) 

hostapd2: libnltiny
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/hostapd clean
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant clean
	echo ` \
		$(MAKE) -s -C hostapd-$(HOSTAPDVERSION)/hostapd MULTICALL=1 dump_cflags; \
		$(MAKE) -s -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant MULTICALL=1 dump_cflags \
	` > hostapd-$(HOSTAPDVERSION)/.cflags

	

	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/hostapd CFLAGS="$$(cat hostapd-$(HOSTAPDVERSION)/.cflags) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) MULTICALL=1 hostapd_cli hostapd_multi.a
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant CFLAGS="$$(cat hostapd-$(HOSTAPDVERSION)/.cflags) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) MULTICALL=1 wpa_cli wpa_supplicant_multi.a
	$(CC) $(COPTS) $(MIPS16_OPT)  -L$(TOP)/nvram  -L$(TOP)/libutils -Wall -ffunction-sections -fdata-sections -Wl,--gc-sections -o hostapd-$(HOSTAPDVERSION)/wpad hostapd-$(HOSTAPDVERSION)/multicall/multicall.c \
		hostapd-$(HOSTAPDVERSION)/hostapd/hostapd_multi.a \
		hostapd-$(HOSTAPDVERSION)/wpa_supplicant/wpa_supplicant_multi.a \
		$(ATH9K_LDFLAGS) -lutils -lnvram

hostapd2-clean:
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/hostapd clean
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant clean

hostapd2-install:
	install -D hostapd-$(HOSTAPDVERSION)/wpad $(INSTALLDIR)/hostapd2/usr/sbin/wpad
ifeq ($(CONFIG_WPS),y)
	install -D hostapd-$(HOSTAPDVERSION)/hostapd/hostapd_cli $(INSTALLDIR)/hostapd2/usr/sbin/hostapd_cli
endif
	cd $(INSTALLDIR)/hostapd2/usr/sbin && ln -sf wpad hostapd
	cd $(INSTALLDIR)/hostapd2/usr/sbin && ln -sf wpad wpa_supplicant

