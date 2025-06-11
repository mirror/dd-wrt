include $(TOP)/.config

# This file implements the GNOME Build API:
# http://people.gnome.org/~walters/docs/build-api.txt

FIRMWAREDIR = /lib/firmware

all:

install:
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)
ifeq ($(CONFIG_IPQ6018),y)
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)/marvell
	cp -v aqr/AQR* $(INSTALLDIR)$(FIRMWAREDIR)/marvell
	mkdir -p $(INSTALLDIR)$(FIRMWAREDIR)/IPQ5018
	cp -v IPQ5018/* $(INSTALLDIR)$(FIRMWAREDIR)/IPQ5018
endif
ifeq ($(CONFIG_MVEBU),y)
	mkdir $(INSTALLDIR)$(FIRMWAREDIR)/mrvl
	cp mrvl/sd8887_uapsta.bin $(INSTALLDIR)$(FIRMWAREDIR)/mrvl
	cp mrvl/sdsd8997_combo_v4.bin $(INSTALLDIR)$(FIRMWAREDIR)/mrvl
	ln -s ../mrvl/sdsd8997_combo_v4.bin $(INSTALLDIR)$(FIRMWAREDIR)/mrvl/sd8997_uapsta.bin
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
