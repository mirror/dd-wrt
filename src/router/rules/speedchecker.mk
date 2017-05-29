speedchecker:
	make -C speedchecker

speedchecker-clean:
	-make -C speedchecker clean

speedchecker-install:
	install -D speedchecker/configs/speedchecker.nvramconfig $(INSTALLDIR)/speedchecker/etc/config/speedchecker.nvramconfig
	install -D speedchecker/configs/speedchecker.webservices $(INSTALLDIR)/speedchecker/etc/config/speedchecker.webservices
