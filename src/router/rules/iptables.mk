iptables-clean:
	-make -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) DO_IPV6=1 clean

iptables:
	make -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) DO_IPV6=1

iptables-devel:
	make -C iptables install-devel LIBDIR=/opt/openwrt/lib/ KERNEL_DIR=$(LINUXDIR)


iptables-install:
ifeq ($(CONFIG_IPTABLES),y)
#	install -d $(INSTALLDIR)/iptables/usr/lib/iptables
#	install iptables/extensions/*.so $(INSTALLDIR)/iptables/usr/lib/iptables
#	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/iptables/*.so
	install -D iptables/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables
ifeq ($(CONFIG_IPV6),y)
	install -D iptables/ip6tables $(INSTALLDIR)/iptables/usr/sbin/ip6tables
endif
	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables
#	install -D iptables/iptables-restore $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
#	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
	ln -sf /usr/sbin/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables-restore
ifeq ($(CONFIG_IPV6),y)
	ln -sf /usr/sbin/ip6tables $(INSTALLDIR)/iptables/usr/sbin/ip6tables-restore
endif
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

iptables-distclean:
	-make -C iptables KERNEL_DIR=$(LINUXDIR) distclean
