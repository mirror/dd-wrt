upnp-clean:
	make -C upnp clean

upnp: nvram
	make -C upnp

upnp-install:
	make -C upnp install

