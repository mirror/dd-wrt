wpa_supplicant: 
ifeq ($(CONFIG_WPA_SUPPLICANT),y)
#	$(MAKE) -C openssl
	$(MAKE) -C hostapd clean
	$(MAKE) -C wpa_supplicant
	$(MAKE) -C hostapd clean
	$(MAKE) -C hostapd
else
	@true
endif

wpa_supplicant-clean:
	$(MAKE) -C wpa_supplicant clean

wpa_supplicant-install:
ifeq ($(CONFIG_WPA_SUPPLICANT),y)
	install -D wpa_supplicant/wpa_supplicant $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_supplicant
	install -D wpa_supplicant/wpa_passphrase $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_passphrase
	install -D wpa_supplicant/wpa_cli $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_supplicant
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_passphrase
	$(STRIP) $(INSTALLDIR)/wpa_supplicant/usr/sbin/wpa_cli
else
	# So that generic rule does not take precedence
	@true
endif

