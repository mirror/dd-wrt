bird: 
	$(MAKE) -C bird

bird-install:
	@true
	install -D bird/bird $(INSTALLDIR)/bird/usr/sbin/bird
	$(STRIP) $(INSTALLDIR)/bird/usr/sbin/bird

bird-configure:
	cd bird && ./configure --target=$(ARCH)-linux --host=$(ARCH) --with-protocols=bgp,rip,static CC=$(ARCH)-linux-uclibc-gcc --prefix=/usr --disable-client --with-sysconfig=sysdep/cf/linux.h --localstatedir=/tmp/bird --sysconfdir=/tmp/bird CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections"

bird-clean:
	$(MAKE) -C bird clean

bird-distclean:
	$(MAKE) -C bird clean
	rm -rf bird/obj/nest bird/obj/filter bird/obj/proto
	rm -f bird/obj/.dir-stamp

