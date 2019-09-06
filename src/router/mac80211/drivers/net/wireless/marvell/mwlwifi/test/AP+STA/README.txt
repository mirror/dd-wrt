<<How to set up AP+STA>>

1. Bring up driver:

	a. iw reg set US
		--> set regulatory domain.
	b. insmod mwlwifi.ko
		--> After module is inserted, physical interfaces phy0 and phy1 and network
				device wlan0 and wlan1 will be created. You can issue "iw dev" to check them.

2. Use interface wlan0/wlan1 as client mode to connect to remote AP:

	wpa_supplicant -B -D nl80211 -i wlan0 -c wpa_supplicant.conf
	wpa_supplicant -B -D nl80211 -i wlan1 -c wpa_supplicant.conf

3. Create local AP:

	a. Create anther interface on phy0/phy1:

	iw phy0 interface add wlan0-1 type managed
	iw phy1 interface add wlan1-1 type managed

	b. Use iw dev to check channel setting for client mode and modify related channel
		setting of hostapd.conf to be complied with currecnt channel setting.

	c. Check MAC address of client interface and add "bssid=02:XX:XX:XX:XX:XX" to hostapd.conf.

	d. Create local AP:

	hostapd -B ./hostapd.conf
		--> Channel setting of hostapd.conf should be complied with current channel setting.
				Please make sure interface of hostapd.conf is complied with the interface you just created.

4. Bridge local AP and STA:

	You need to use relayd to bridge them. Same as LAN and STA.
