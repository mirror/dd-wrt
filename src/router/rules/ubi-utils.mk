
ubi-utils:
	$(MAKE) -C ubi-utils

ubi-utils-clean:
	$(MAKE) -C ubi-utils clean

ubi-utils-install:
	$(MAKE) -C ubi-utils install DESTDIR=$(INSTALLDIR)/ubi-utils
	rm -rf $(INSTALLDIR)/ubi-utils/usr/share