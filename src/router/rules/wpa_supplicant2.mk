wpa_supplicant2: 
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
#	$(MAKE) -C openssl
	$(MAKE) -C hostapd2/hostapd clean
	$(MAKE) -C hostapd2/wpa_supplicant
	$(MAKE) -C hostapd2/hostapd clean
	$(MAKE) -C hostapd2/hostapd
else
	@true
endif

wpa_supplicant2-clean:
	$(MAKE) -C hostapd2/wpa_supplicant clean

wpa_supplicant2-install:
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
	install -D hostapd2/wpa_supplicant/wpa_supplicant $(INSTALLDIR)/wpa_supplicant2/usr/sbin/wpa_supplicant
#	install -D hostapd2/wpa_supplicant/wpa_passphrase $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_passphrase
#	install -D hostapd2/wpa_supplicant/wpa_cli $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
	$(STRIP) $(INSTALLDIR)/wpa_supplicant2/usr/sbin/wpa_supplicant
else
	# So that generic rule does not take precedence
	@true
endif

