wpa_supplicant2: 
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
	$(MAKE) -C hostapd2/wpa_supplicant clean
	$(MAKE) -C hostapd2/wpa_supplicant
else
	@true
endif

wpa_supplicant2-clean:
	$(MAKE) -C hostapd2/wpa_supplicant clean

wpa_supplicant2-install:
ifeq ($(CONFIG_WPA_SUPPLICANT2),y)
	install -D hostapd2/wpa_supplicant/wpa_supplicant $(INSTALLDIR)/wpa_supplicant2/usr/sbin/wpa_supplicant
	$(STRIP) $(INSTALLDIR)/wpa_supplicant2/usr/sbin/wpa_supplicant
else
	# So that generic rule does not take precedence
	@true
endif

