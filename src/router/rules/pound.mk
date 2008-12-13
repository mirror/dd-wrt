obj-$(CONFIG_POUND) += pound


pound-configure:
	cd pound && ./configure --with-ssl=$(TOP)/openssl --host=$(ARCH)-linux CC=$(ARCH)-linux-uclibc-gcc LDFLAGS="-Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl" CFLAGS="-Drpl_malloc=malloc -ffunction-sections -fdata-sections -Wl,--gc-sections $(COPTS) -L$(TOP)/openssl"

pound-clean:
	make -C pound clean

pound:
	make -C pound

pound-install:
	@true
