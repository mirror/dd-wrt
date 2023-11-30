ifeq ($(CONFIG_ATH9K),y)
ATH9K_CFLAGS=-I$(TOP)/libnl-tiny/include\
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE
ATH9K_LDFLAGS=-L$(TOP)/libnl-tiny/ -lm -lnl-tiny
TINY=libnltiny
endif

ifeq ($(CONFIG_DRIVER_WIRED),y)
ATH9K_LDFLAGS +=-L$(TOP)/libnfnetlink/src/.libs -lnfnetlink
ATH9K_LDFLAGS +=-L$(TOP)/libnetfilter_log/src/.libs -lnetfilter_log
endif
ifeq ($(CONFIG_WPA3),y)
ifndef $(HOSTAPDVERSION)
HOSTAPDVERSION=2018-07-08
endif
ifeq ($(CONFIG_ATH11K),y)
ifndef $(HOSTAPDVERSION)
HOSTAPDVERSION=2023-09-08
endif
endif
else
ifndef $(HOSTAPDVERSION)
HOSTAPDVERSION=2017-08-24
endif
endif

ifeq ($(KERNELVERSION),6.1)
HOSTAPDVERSION=2023-09-08
endif

ifeq ($(KERNELVERSION),6.1)
ATH9K_CFLAGS += $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/_staging/usr/include 
ATH9K_LDFLAGS += $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/_staging/usr/lib -lubox -lubus
else
ATH9K_CFLAGS += $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections
ATH9K_LDFLAGS += $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(CONFIG_WPA3),y)
ifeq ($(CONFIG_OPENSSL),y)
ATH9K_LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl -L$(TOP)/libucontext -lucontext
else
ATH9K_LDFLAGS += -L$(TOP)/wolfssl/standard/src/.libs -lwolfssl
endif
endif

hostapd2: $(TINY) nvram ubus
	cp shared/nl80211.h hostapd-$(HOSTAPDVERSION)/src/drivers/nl80211_copy.h
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/hostapd clean
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant clean
	echo ` \
		$(MAKE) -s -C hostapd-$(HOSTAPDVERSION)/hostapd MULTICALL=1 dump_cflags; \
		$(MAKE) -s -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant MULTICALL=1 dump_cflags \
	` > hostapd-$(HOSTAPDVERSION)/.cflags

	

	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/hostapd CFLAGS="$$(cat hostapd-$(HOSTAPDVERSION)/.cflags) $(LTO) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) MULTICALL=1 hostapd_cli hostapd_multi.a
	$(MAKE) -C hostapd-$(HOSTAPDVERSION)/wpa_supplicant CFLAGS="$$(cat hostapd-$(HOSTAPDVERSION)/.cflags) $(LTO) $(ATH9K_CFLAGS)" CONFIG_ATH9K=$(CONFIG_ATH9K) MULTICALL=1 wpa_cli wpa_supplicant_multi.a
	$(CC) $(COPTS) $(MIPS16_OPT) $(LDLTO) -L$(TOP)/nvram  -L$(TOP)/libutils -Wall -ffunction-sections -fdata-sections -Wl,--gc-sections -o hostapd-$(HOSTAPDVERSION)/wpad hostapd-$(HOSTAPDVERSION)/multicall/multicall.c \
		hostapd-$(HOSTAPDVERSION)/hostapd/hostapd_multi.a \
		hostapd-$(HOSTAPDVERSION)/wpa_supplicant/wpa_supplicant_multi.a \
		$(ATH9K_LDFLAGS) -lutils -lqos -lshutils -lnvram

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
ifeq ($(CONFIG_WPA3),y)
	install -D hostapd-$(HOSTAPDVERSION)/hostapd/hostapd_cli $(INSTALLDIR)/hostapd2/usr/sbin/hostapd_cli
endif

