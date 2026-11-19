services-clean:
	make -C services clean

services: nvram shared
	make -C services CONFIG_SSID_PROTECTION=$(CONFIG_SSID_PROTECTION) CONFIG_OPENSSL3=$(CONFIG_OPENSSL3) CONFIG_PORTSCAN=$(CONFIG_PORTSCAN)

services-install:
	make -C services install

