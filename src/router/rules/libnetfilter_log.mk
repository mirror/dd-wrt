libnetfilter_log-configure: libnfnetlink
	cd libnetfilter_log && \
	LIBNFNETLINK_CFLAGS="-I$(TOP)/libnfnetlink/include" \
	LIBNFNETLINK_LIBS="-L$(TOP)/libnfnetlink/src/.libs" \
	./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--libdir=$(TOP)/libnfnetlink/src/.libs
		
		#CFLAGS="$(COPTS) -fPIC -DNEED_PRINTF -I$(TOP)/iptables/include/libipq/" LDFLAGS="-L$(TOP)/iptables/libipq"


libnetfilter_log: libnfnetlink
	$(MAKE) -C libnetfilter_log CFLAGS="$(COPTS) -DNEED_PRINTF"
	$(MAKE) -C libnetfilter_log/utils nfulnl_test CFLAGS="$(COPTS) -DNEED_PRINTF"
	#$(MAKE) -C libnetfilter_log/utils CFLAGS="$(COPTS) -DNEED_PRINTF"

libnetfilter_log-install:
	install -D libnetfilter_log/src/.libs/libnetfilter_log.so.1 $(INSTALLDIR)/libnetfilter_log/usr/lib/libnetfilter_log.so.1
