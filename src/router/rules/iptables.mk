iptables-clean:
	$(MAKE) -C iptables.linksys clean
	$(MAKE) -C iptables clean

iptables:
	$(MAKE) -j 4 -C iptables.linksys clean
	$(MAKE) -j 4 -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR)

iptables-devel:
	$(MAKE) -j 4 -C iptables install-devel LIBDIR=/opt/openwrt/lib/ KERNEL_DIR=$(LINUXDIR)


iptables-install:
ifeq ($(CONFIG_IPTABLES),y)
#	install -d $(INSTALLDIR)/iptables/usr/lib/iptables
#	install iptables/extensions/*.so $(INSTALLDIR)/iptables/usr/lib/iptables
#	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/iptables/*.so
	install -D iptables/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables
#	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables
#	install -D iptables/iptables-restore $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
#	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
	ln -sf /usr/sbin/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
#	ln -sf /usr/sbin/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables-save

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

iptables-clean:
	-$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) clean

iptables-distclean:
	-$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) distclean
