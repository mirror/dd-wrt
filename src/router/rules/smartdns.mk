smartdns-configure:
	@true

smartdns-clean:
	$(MAKE) -C smartdns/src clean 

smartdns: openssl
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl"

smartdns-install:
	install -D smartdns/src/smartdns $(INSTALLDIR)/smartdns/usr/sbin/smartdns

