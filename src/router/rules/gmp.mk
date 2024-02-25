
ifneq ($(CONFIG_IPV6),y)
gmp-configure:
	cd gmp && autoreconf --force --install --symlink
	cd gmp && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=/usr/lib \
		--disable-shared \
		--enable-static \
		--enable-assembly \
		--disable-assert \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/iptables/include/libipq/ -ffunction-sections -fdata-sections" LDFLAGS="-L$(TOP)/iptables/libipq $(LDLTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
else
gmp-configure:
	cd gmp && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=/usr/lib \
		--disable-shared \
		--enable-static \
		--enable-assembly \
		--disable-assert \
		CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/iptables-new/include/libipq/ -ffunction-sections -fdata-sections" LDFLAGS="-L$(TOP)/iptables-new/libipq/.libs $(LDLTO)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
endif

gmp:
	$(MAKE) -C gmp CFLAGS="$(COPTS) $(LTO)"

gmp-install:
	@true
#	$(MAKE) -C gmp install DESTDIR=$(INSTALLDIR)/gmp
#	rm -rf $(INSTALLDIR)/gmp/usr/share
#	rm -rf $(INSTALLDIR)/gmp/usr/include
#	mkdir -p $(INSTALLDIR)/gmp/usr/lib
#	-mv $(INSTALLDIR)/gmp/usr/lib64/* $(INSTALLDIR)/gmp/usr/lib  
#	rm -f $(INSTALLDIR)/gmp/usr/lib/libgmp.a
#	rm -f $(INSTALLDIR)/gmp/usr/lib/libgmp.la
#	rm -rf $(INSTALLDIR)/gmp/usr/lib64



