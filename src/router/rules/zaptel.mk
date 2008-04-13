zaptel:
	cd zaptel && $(MAKE) KSRC=$(LINUXDIR)

zaptel-clean:
	cd zaptel && $(MAKE) clean

zaptel-install:
	cd zaptel && $(MAKE) KSRC=$(LINUXDIR) DESTDIR=$(INSTALLDIR)/zaptel install-modules
