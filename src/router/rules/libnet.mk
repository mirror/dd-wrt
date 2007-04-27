libnet:
	-mkdir -p libnet/lib
	$(MAKE) -C libnet CC=$(CC) AR=$(AR) RANLIB=$(RANLIB)

libnet-install:
	@true
