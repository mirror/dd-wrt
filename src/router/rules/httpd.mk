
httpd: nvram shared hsiab matrixssl wolfssl www wireless-tools
	make -C httpd CONFIG_SSID_PROTECTION=$(CONFIG_SSID_PROTECTION) CONFIG_PORTSCAN=$(CONFIG_PORTSCAN)

httpd-clean:
	make -C httpd clean
	-rm -f $(TOP)/register/*.o


