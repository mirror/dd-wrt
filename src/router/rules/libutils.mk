ifeq ($(CONFIG_SAMBA3),y)
	JANSSON=jansson
endif
ifeq ($(CONFIG_FTP),y)
	JANSSON=jansson
endif
ifeq ($(CONFIG_MINIDLNA),y)
	JANSSON=jansson
endif
ifeq ($(CONFIG_ATH9K),y)
	TINY=libnltiny
endif

libutils-configure: jansson-configure
	@true

libutils-clean:
	make -C libutils clean

libutils: nvram $(TINY)  $(JANSSON) wireless-tools
	make -C libutils CONFIG_SSID_PROTECTION=$(CONFIG_SSID_PROTECTION) CONFIG_PORTSCAN=$(CONFIG_PORTSCAN)

libutils-install:
	make -C libutils install

