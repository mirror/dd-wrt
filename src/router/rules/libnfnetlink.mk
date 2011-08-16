libnfnetlink-configure:
	cd libnfnetlink && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu 

		#CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


libnfnetlink:
	$(MAKE) -C libnfnetlink CFLAGS="$(COPTS) -DNEED_PRINTF"

libnfnetlink-install:
	@true
