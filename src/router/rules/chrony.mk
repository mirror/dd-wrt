chrony-configure:
	cd chrony && \
	CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" \
	./configure --host-machine=$(ARCH)-linux \
	--host-system=Linux \
	--prefix=/usr \
	--disable-readline \
	--disable-rtc \
	--disable-sechash \
	--sysconfdir=/etc/chrony \

chrony:
	make   -C chrony

chrony-clean:
	make   -C chrony clean

chrony-install:
	make   -C chrony install DESTDIR=$(INSTALLDIR)/chrony
	mkdir -p $(INSTALLDIR)/chrony/tmp/var/lib/chrony
	ln -s /tmp/chrony.conf $(INSTALLDIR)/chrony/etc/chrony/chrony.conf
	rm -rf $(INSTALLDIR)/chrony/usr/share
	rm -rf $(INSTALLDIR)/chrony/var
