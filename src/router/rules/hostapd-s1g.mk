hostapd-s1g:
	cp -uvf shared/nl80211.h hostapd_s1g/src/drivers/nl80211_copy.h
	$(MAKE) -C hostapd_s1g/hostapd 

hostapd-s1g-clean:
	$(MAKE) -C hostapd_s1g/hostapd clean

hostapd-s1g-install:
	install -D hostapd_s1g/hostapd/hostapd_s1g $(INSTALLDIR)/hostapd-s1g/usr/sbin/hostapd_s1g

