gmp-configure:
	cd gmp && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


gmp:
	$(MAKE) -C gmp CFLAGS="$(COPTS) -fPIC"

gmp-install:
	$(MAKE) -C gmp install DESTDIR=$(INSTALLDIR)/gmp
	rm -rf $(INSTALLDIR)/gmp/usr/share
	rm -rf $(INSTALLDIR)/gmp/usr/include
	mkdir -p $(INSTALLDIR)/gmp/usr/lib
	-mv $(INSTALLDIR)/gmp/usr/lib64/* $(INSTALLDIR)/gmp/usr/lib  
	rm -f $(INSTALLDIR)/gmp/usr/lib/libgmp.a
	rm -f $(INSTALLDIR)/gmp/usr/lib/libgmp.la
	rm -rf $(INSTALLDIR)/gmp/usr/lib64



