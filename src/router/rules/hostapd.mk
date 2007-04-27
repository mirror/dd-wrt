hostapd: 
#	$(MAKE) -C openssl
	$(MAKE) -C hostapd clean
	$(MAKE) -C wpa_supplicant
	$(MAKE) -C hostapd clean
	$(MAKE) -C hostapd

hostapd-clean:
	$(MAKE) -C hostapd clean

hostapd-install:
	$(MAKE) -C hostapd install

