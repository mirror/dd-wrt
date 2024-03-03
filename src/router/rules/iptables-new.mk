iptables-new-configure:
	cd iptables-new && ./autogen.sh
	cd iptables-new && ./configure --host=$(ARCH)-linux --prefix=/usr --libdir=/usr/lib --with-kernel=$(LINUXDIR) --enable-libipq --disable-shared --enable-static --disable-nftables \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

iptables-new-clean:
	-make -C iptables-new clean

iptables-new:
#ifneq ($(CONFIG_NOMESSAGE),y)
	make -C iptables-new CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF"
#else
#	make -C iptables-new CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon"
#endif



iptables-new-install:
ifeq ($(CONFIG_IPTABLES),y)
	make -C iptables-new install DESTDIR=$(INSTALLDIR)/iptables-new
	rm -rf $(INSTALLDIR)/iptables-new/usr/include
	rm -rf $(INSTALLDIR)/iptables-new/usr/lib
	rm -rf $(INSTALLDIR)/iptables-new/usr/share
        ifeq ($(CONFIG_L7),y)
		  install -d $(INSTALLDIR)/iptables-new/etc/l7-protocols
		  cp -rp l7/* $(INSTALLDIR)/iptables-new/etc/l7-protocols/
		  rm -f $(INSTALLDIR)/iptables-new/etc/l7-protocols/CHANGELOG
		  rm -f $(INSTALLDIR)/iptables-new/etc/l7-protocols/LICENSE
		  rm -f $(INSTALLDIR)/iptables-new/etc/l7-protocols/README
		  rm -f $(INSTALLDIR)/iptables-new/etc/l7-protocols/HOWTO
        endif
else
        # So that generic rule does not take precedence
	@true
endif

iptables-distclean:
	-make -C iptables-new distclean



iptables-ipt-clean:
	-make -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) DO_IPV6=1 clean

iptables-ipt:
	make -C iptables DO_MULTI=1 BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) DO_IPV6=1

iptables-ipt-install:
	@true