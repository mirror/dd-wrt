# This file implements the GNOME Build API:
# http://people.gnome.org/~walters/docs/build-api.txt

FIRMWAREDIR = /lib/firmware

all:

install:
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)
	cp -r rtl_nic $(INSTALLDIR)$(FIRMWAREDIR)
	-rm -f $(INSTALLDIR)/usbdux/*dux $(INSTALLDIR)/*/*.asm
	-rm $(INSTALLDIR)$(FIRMWAREDIR)/{WHENCE,LICENSE.*,LICENCE.*}
