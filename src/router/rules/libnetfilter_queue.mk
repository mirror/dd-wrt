libnetfilter_queue-configure: libnfnetlink-configure libnfnetlink
	export LIBNFNETLINK_CFLAGS="-I$(TOP)/libnfnetlink/include" ;\
	export LIBNFNETLINK_LIBS="-L$(TOP)/libnfnetlink/src/.libs" ; \
	cd libnetfilter_queue && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--libdir=$(TOP)/libnetfilter_queue/src/.libs \
		--prefix=/usr \
		--disable-shared \
		--enable-static 


libnetfilter_queue: libnfnetlink
	$(MAKE) -C libnetfilter_queue CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/libnfnetlink/include" LDFLAGS="-L$(TOP)/libnfnetlink/src/.libs"

libnetfilter_queue-install:
	@true	
#	install -D libnetfilter_queue/src/.libs/libnetfilter_queue.so.1 $(INSTALLDIR)/libnetfilter_queue/usr/lib/libnetfilter_queue.so.1
