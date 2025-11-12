opennds-configure:
	@true

opennds: libmicrohttpd
	$(MAKE) -C opennds \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -I$(TOP)/libmicrohttpd/src/include" \
	    LDFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF  -ffunction-sections -fdata-sections -Wl,--gc-sections $(LTO) -L$(TOP)/libmicrohttpd/src/microhttpd/.libs"


opennds-install:
	$(MAKE) -C opennds install DESTDIR=$(INSTALLDIR)/opennds

