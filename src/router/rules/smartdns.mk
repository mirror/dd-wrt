smartdns-configure:
	@true

smartdns-clean:
	$(MAKE) -C smartdns/src clean 

smartdns: openssl
ifeq ($(CONFIG_OPENSSL),y)
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl" clean
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl" SSL=yes
else
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl" clean
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/openssl/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(TOP)/openssl"
endif

smartdns-install:
	install -D smartdns/src/smartdns $(INSTALLDIR)/smartdns/usr/sbin/smartdns

