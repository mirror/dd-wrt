jansson-configure:
	if ! test -e "jansson/Makefile"; then cd jansson && ./configure --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC"; fi

jansson: jansson-configure
	$(MAKE) -C jansson

jansson-clean:
	if test -e "jansson/Makefile"; then make -C jansson clean; fi
	@true

jansson-install:
	@true
