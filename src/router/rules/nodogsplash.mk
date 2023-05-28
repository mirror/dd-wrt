
nodogsplash-configure:
	@true

nodogsplash:
	$(MAKE) -C nodogsplash \
	    CFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(LTO) -I$(TOP)/libmicrohttpd/src/include" \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(LTO) -L$(TOP)/libmicrohttpd/src/microhttpd/.libs"


nodogsplash-install:
	@true
