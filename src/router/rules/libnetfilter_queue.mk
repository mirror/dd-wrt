libnetfilter_queue-configure: libnfnetlink
	export LIBNFNETLINK_CFLAGS="-I$(TOP)/libnfnetlink/include" ;\
	export LIBNFNETLINK_LIBS="-L$(TOP)/libnfnetlink/src/.libs" ; \
	cd libnetfilter_queue && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu  
		#CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/libnfnetlink/include/libnfnetlink" LDFLAGS="-L$(TOP)/libnfnetlink/src/.libs"


libnetfilter_queue: libnfnetlink
	$(MAKE) -C libnetfilter_queue CFLAGS="$(COPTS) -DNEED_PRINTF -I$(TOP)/libnfnetlink/include" LDFLAGS="-L$(TOP)/libnfnetlink/src/.libs"

libdnet-install:
	@true
