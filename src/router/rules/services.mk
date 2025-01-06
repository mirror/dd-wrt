services-clean:
	make -C services clean

services: nvram shared
	make -C services CONFIG_SSID_PROTECTION=$(CONFIG_SSID_PROTECTION)

services-install:
	make -C services install

