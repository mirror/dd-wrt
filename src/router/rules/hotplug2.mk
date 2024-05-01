MAKE_FLAGS += STATIC_WORKER="single"
hotplug2:
	$(MAKE) $(MAKE_FLAGS) COPTS="$(COPTS) $(MIPS16_OPT) $(LTO) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" -C hotplug2 

hotplug2-clean:
	$(MAKE) -C hotplug2 clean

hotplug2-install:
	install -D hotplug2/hotplug2 $(INSTALLDIR)/hotplug2/sbin/hotplug2
	mkdir -p $(INSTALLDIR)/hotplug2/etc/
	cd hotplug2/config/etc/ ; cp -av * $(INSTALLDIR)/hotplug2/etc/

udev:
	$(MAKE) COPTS="$(COPTS)  $(MIPS16_OPT)  $(LTO) $(THUMB) -ffunction-sections -fdata-sections -Wl,--gc-sections" -C udev udevtrigger

udev-clean:
	$(MAKE) -C udev clean

udev-install:
	install -D udev/udevtrigger $(INSTALLDIR)/udev/sbin/udevtrigger
