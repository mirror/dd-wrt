smartdns-configure:
	@true

smartdns-clean:
	$(MAKE) -C smartdns/src clean 

smartdns: openssl
ifeq ($(CONFIG_OPENSSL),y)
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(SSLPATH)/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(SSLPATH)" clean
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(SSLPATH)/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(SSLPATH)" SSL=yes
else
ifeq ($(CONFIG_WOLFSSL),y)
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(WOLFSSL_SSLPATH)/standard/src/.libs -lwolfssl" clean
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DOPENSSL_EXTRA -I$(WOLFSSL_SSLPATH) -I$(WOLFSSL_SSLPATH)/standard -I$(WOLFSSL_SSLPATH)/standard/wolfssl  -I$(WOLFSSL_SSLPATH)/wolfssl" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(WOLFSSL_SSLPATH)/standard/src/.libs -lwolfssl" WOLFSSL=yes
else
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(SSLPATH)/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(SSLPATH)" clean
	$(MAKE) -C smartdns/src CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(SSLPATH)/include" LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections  -L$(SSLPATH)"
endif
endif
smartdns-install:
	install -D smartdns/src/smartdns $(INSTALLDIR)/smartdns/usr/sbin/smartdns

