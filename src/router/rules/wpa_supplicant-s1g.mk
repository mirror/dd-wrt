wpa_supplicant-s1g:
	cp -uvf shared/nl80211.h wpa_supplicant_s1g/src/drivers/nl80211_copy.h
	$(MAKE) -C wpa_supplicant_s1g/wpa_supplicant

wpa_supplicant-s1g-clean:
	$(MAKE) -C wpa_supplicant_s1g/wpa_supplicant clean

wpa_supplicant-s1g-install:
	install -D wpa_supplicant_s1g/wpa_supplicant/wpa_supplicant_s1g $(INSTALLDIR)/wpa_supplicant-s1g/usr/sbin/wpa_supplicant_s1g

