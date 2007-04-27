ebtables:
	$(MAKE) -C ebtables static BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) KERNEL_INCLUDES=$(LINUXDIR)/include

ebtables-install:
ifeq ($(CONFIG_EBTABLES),y)
	install -D ebtables/ebtables $(INSTALLDIR)/ebtables/usr/sbin/ebtables
else
        # So that generic rule does not take precedence
	@true
endif

ebtables-clean:
	-$(MAKE) -C ebtables KERNEL_DIR=$(LINUXDIR) clean
