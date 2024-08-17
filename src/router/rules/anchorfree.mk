libevent-af:
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(SSLPATH)" \
	$(MAKE) -C libevent2-anchorfree

libevent-af-install:
	@true


libevent-af-clean:
	$(MAKE) -C libevent2-anchorfree clean


libevent-af-configure:
	cd libevent2-anchorfree && ./autogen.sh
	cd libevent2-anchorfree && ./configure  --enable-static --disable-debug-mode --prefix=/usr ac_cv_host=$(ARCH)-uclibc-linux --libdir=/usr/lib --target=$(ARCH)-linux --host=$(ARCH) CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(SSLPATH)" 
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) $(MIPS16_OPT) -I$(SSLPATH)/include -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(SSLPATH)" \
	$(MAKE) -C libevent2-anchorfree





ifneq ($(CONFIG_MUSL),y)
HYDRA_OPTS:= \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	LD="$(ARCH)-linux-uclibc-ld" \
	EXTRA_CFLAGS+="-ffunction-sections -fdata-sections -DOPENWRT -DDISABLE_VIPER_BSS_RESET $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fno-strict-aliasing" \
	EXTRA_CFLAGS+="-I$(TOP)/jansson/src" \
	EXTRA_CFLAGS+="-I$(TOP)/libevent2-anchorfree/include" \
	EXTRA_CFLAGS+="-I$(SSLPATH)/include" \
	EXTRA_CFLAGS+="-I$(TOP)/libevent2-anchorfree/compat" \
	EXTRA_CFLAGS+="-I$(TOP)/zlib/include" \
	EXTRA_LDFLAGS+="$(TOP)/libevent2-anchorfree/.libs/libevent.a" \
	EXTRA_LDFLAGS+="$(TOP)/libevent2-anchorfree/.libs/libevent_pthreads.a" \
	EXTRA_LDFLAGS+="$(TOP)/jansson/src/.libs/libjansson.a" \
	EXTRA_LDFLAGS+="-L$(TOP)/zlib -lz" \
	EXTRA_LDFLAGS+="-L$(SSLPATH) -lssl -lcrypto -lpthread" \
	EXTRA_LDFLAGS+="-Wl,--gc-sections" \
	OPENWRT=yes \
	PLATFORM=openwrt
else
HYDRA_OPTS:= \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	LD="$(ARCH)-linux-uclibc-ld" \
	EXTRA_CFLAGS+="-ffunction-sections -fdata-sections -DOPENWRT -DDISABLE_VIPER_BSS_RESET $(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -fno-strict-aliasing" \
	EXTRA_CFLAGS+="-I$(TOP)/jansson/src" \
	EXTRA_CFLAGS+="-I$(TOP)/libevent2-anchorfree/include" \
	EXTRA_CFLAGS+="-I$(SSLPATH)/include" \
	EXTRA_CFLAGS+="-I$(TOP)/libevent2-anchorfree/compat" \
	EXTRA_CFLAGS+="-I$(TOP)/zlib/include" \
	EXTRA_LDFLAGS+="$(TOP)/libevent2-anchorfree/.libs/libevent.a" \
	EXTRA_LDFLAGS+="$(TOP)/libevent2-anchorfree/.libs/libevent_pthreads.a" \
	EXTRA_LDFLAGS+="$(TOP)/jansson/src/.libs/libjansson.a" \
	EXTRA_LDFLAGS+="-L$(TOP)/zlib -lz" \
	EXTRA_LDFLAGS+="-L$(SSLPATH) -lssl -lcrypto" \
	EXTRA_LDFLAGS+="-Wl,--gc-sections" \
	OPENWRT=yes \
	PLATFORM=openwrt

endif

hydra: 
	$(MAKE) -C hydra/sd $(HYDRA_OPTS)
	$(MAKE) -C hydra/tranceport $(HYDRA_OPTS)
	$(MAKE) -C hydra/viper $(HYDRA_OPTS)
	$(MAKE) -C hydra/vpr_stats $(HYDRA_OPTS)
	$(MAKE) -C hydra/proxy_plugin $(HYDRA_OPTS)
	$(MAKE) -C hydra/ecc-ser $(HYDRA_OPTS)
	$(MAKE) -C hydra/afvpn $(HYDRA_OPTS)

	
hydra-clean:
	$(MAKE) -C hydra/sd clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/tranceport clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/viper clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/vpr_stats clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/proxy_plugin clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/ecc-ser clean $(HYDRA_OPTS)
	$(MAKE) -C hydra/afvpn clean $(HYDRA_OPTS)

hydra-install:
	mkdir -p $(INSTALLDIR)/hydra/usr/sbin
	cp hydra/afvpn/client.openwrt $(INSTALLDIR)/hydra/usr/sbin/afvpn
	cp -urv hydra/config_dd-wrt/* $(INSTALLDIR)/hydra