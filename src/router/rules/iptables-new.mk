iptables-configure:
	cd iptables-new && ./autogen.sh
	cd iptables-new && ./configure --host=$(ARCH)-linux-elf --prefix=/usr --libdir=/usr/lib --with-kernel=$(LINUXDIR) --disable-nftables \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

iptables-clean:
	-$(MAKE) -C iptables-new clean

iptables:
	-$(MAKE) -C iptables-new



iptables-install:
ifeq ($(CONFIG_IPTABLES),y)
	-$(MAKE) -C iptables-new install DESTDIR=$(INSTALLDIR)/iptables
	rm -rf $(INSTALLDIR)/iptables/usr/include
	rm -rf $(INSTALLDIR)/iptables/usr/lib
	rm -rf $(INSTALLDIR)/iptables/usr/share
        ifeq ($(CONFIG_L7),y)
		  install -d $(INSTALLDIR)/iptables/etc/l7-protocols
		  cp -rp l7/* $(INSTALLDIR)/iptables/etc/l7-protocols/
		  rm -f $(INSTALLDIR)/iptables/etc/l7-protocols/CHANGELOG
		  rm -f $(INSTALLDIR)/iptables/etc/l7-protocols/LICENSE
		  rm -f $(INSTALLDIR)/iptables/etc/l7-protocols/README
		  rm -f $(INSTALLDIR)/iptables/etc/l7-protocols/HOWTO
        endif
else
        # So that generic rule does not take precedence
	@true
endif

iptables-distclean:
	-$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) distclean
