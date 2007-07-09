hostapd2: 
#	$(MAKE) -C openssl
	$(MAKE) -C hostapd2/hostapd clean
	$(MAKE) -C wpa_supplicant
	$(MAKE) -C hostapd2/hostapd clean
	$(MAKE) -C hostapd2/hostapd

hostapd2-clean:
	$(MAKE) -C hostapd2/hostapd clean

hostapd2-install:
	$(MAKE) -C hostapd2/hostapd install

