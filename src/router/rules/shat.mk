shat:
	make -C shat/src

shat-install:
	install -D shat/src/shatd $(INSTALLDIR)/shat/usr/sbin/shatd
	install -D shat/src/shatc $(INSTALLDIR)/shat/usr/sbin/shatc
	install -D shat/config/shat.webhotspot $(INSTALLDIR)/shat/etc/config/shat.webhotspot
	install -D shat/config/shat.nvramconfig $(INSTALLDIR)/shat/etc/config/shat.nvramconfig
	install -D shat/config/shat.startup $(INSTALLDIR)/shat/etc/config/shat.startup

	
shat-clean:
	make  -C shat/src clean