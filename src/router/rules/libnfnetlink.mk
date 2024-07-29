libnfnetlink-configure:
	cd libnfnetlink && ./configure \
		--build=$(ARCH)-linux \
		--host=$(ARCH)-linux-gnu \
		--prefix=/usr \
		--enable-shared \
		--disable-static \
		--libdir=$(TOP)/libnfnetlink/src/.libs

libnfnetlink:
	$(MAKE) -C libnfnetlink CFLAGS="$(COPTS) $(MIPS16_OPT) -Wno-int-conversion -D_GNU_SOURCE"

libnfnetlink-install:
	install -D libnfnetlink/src/.libs/libnfnetlink.so.0 $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0
