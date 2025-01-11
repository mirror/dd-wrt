include $(TOP)/.config

# This file implements the GNOME Build API:
# http://people.gnome.org/~walters/docs/build-api.txt

FIRMWAREDIR = /lib/firmware

all:

install:
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)
ifeq ($(CONFIG_IPQ6018),y)
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)/marvell
	cp -v aqr/* $(INSTALLDIR)$(FIRMWAREDIR)/marvell
endif
ifeq ($(ARCH),i386)
	cp -vr rtl_nic $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr amd $(INSTALLDIR)$(FIRMWAREDIR)
#	cp -vr amd-ucode $(INSTALLDIR)$(FIRMWAREDIR)
#	cp -vr intel-ucode $(INSTALLDIR)$(FIRMWAREDIR)
endif
ifeq ($(ARCH),x86_64)
	cp -vr rtl_nic $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr amd $(INSTALLDIR)$(FIRMWAREDIR)
#	cp -vr amd-ucode $(INSTALLDIR)$(FIRMWAREDIR)
#	cp -vr intel-ucode $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr nvidia $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr amdgpu $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr radeon $(INSTALLDIR)$(FIRMWAREDIR)
	cp -vr i915 $(INSTALLDIR)$(FIRMWAREDIR)
endif
	-rm -f $(INSTALLDIR)/usbdux/*dux $(INSTALLDIR)/*/*.asm
	-rm $(INSTALLDIR)$(FIRMWAREDIR)/{WHENCE,LICENSE.*,LICENCE.*}

clean:
