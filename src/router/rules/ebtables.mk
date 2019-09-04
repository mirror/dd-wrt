ebtables:
ifeq ($(CONFIG_HOTPLUG2),y)
	-cp -f ebtables-2.0.9/ebtables-standalone.c.use ebtables-2.0.9/ebtables-standalone.c
	$(MAKE) -C ebtables-2.0.9 static BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR)
else
	-cp -f ebtables/ebtables-standalone.c.use ebtables/ebtables-standalone.c
	$(MAKE) -C ebtables static BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) KERNEL_INCLUDES=$(TOP)/kernel_headers/$(KERNELRELEASE)/include
endif

ebtables-install:
ifeq ($(CONFIG_EBTABLES),y)
ifeq ($(CONFIG_HOTPLUG2),y)
	install -D ebtables-2.0.9/static $(INSTALLDIR)/ebtables/usr/sbin/ebtables
else
	install -D ebtables/ebtables $(INSTALLDIR)/ebtables/usr/sbin/ebtables

endif
else
        # So that generic rule does not take precedence
	@true
endif

ebtables-clean:
ifeq ($(CONFIG_HOTPLUG2),y)
	-$(MAKE) -C ebtables-2.0.9 KERNEL_DIR=$(LINUXDIR) clean
else
	-$(MAKE) -C ebtables KERNEL_DIR=$(LINUXDIR) clean
endif