upnp-clean:
	make -C upnp clean

upnp: nvram netconf
	make -C upnp

upnp-install:
	make -C upnp install

