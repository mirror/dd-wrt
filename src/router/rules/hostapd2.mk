hostapd2: 
	$(MAKE) -C hostapd2/hostapd -j1 clean
	$(MAKE) -C hostapd2/hostapd -j1

hostapd2-clean:
	$(MAKE) -C hostapd2/hostapd clean

hostapd2-install:
	$(MAKE) -C hostapd2/hostapd install

