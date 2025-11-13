iptables-new-configure:
	cd iptables-new && ./autogen.sh
	mkdir -p iptables-new/normal
	mkdir -p iptables-new/nftables
	cd iptables-new/normal && ../configure --host=$(ARCH)-linux --prefix=/usr --libdir=/usr/lib --with-kernel=$(LINUXDIR) --enable-libipq --disable-shared --enable-static --disable-nftables \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	make -C iptables-new/normal CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF"

	cd iptables-new/nftables && ../configure --host=$(ARCH)-linux --prefix=/usr --libdir=/usr/lib --with-kernel=$(LINUXDIR) --enable-libipq --disable-shared --enable-static \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF" \
		LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		libnftnl_CFLAGS="-I$(TOP)/libnftnl/include -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" \
		libnftnl_LIBS="-L$(TOP)/libnftnl/src/.libs -lnftnl" \
		libmnl_CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/libmnl/include" \
		libmnl_LIBS="$(LDLTO) $(COPTS) $(MIPS16_OPT) -L$(TOP)/libmnl/src/.libs -lmnl" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

iptables-new-clean:
	-make -C iptables-new/normal clean
	-make -C iptables-new/nftables clean

iptables-new:
	make -C iptables-new/normal CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF"
ifeq ($(CONFIG_NFTABLES),y)
	make -C iptables-new/nftables CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -fcommon -DNEED_PRINTF"
endif



iptables-new-install:
ifeq ($(CONFIG_IPTABLES),y)
ifeq ($(CONFIG_NFTABLES),y)
	make -C iptables-new/nftables install DESTDIR=$(INSTALLDIR)/iptables-new
else
	make -C iptables-new/normal install DESTDIR=$(INSTALLDIR)/iptables-new
endif
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