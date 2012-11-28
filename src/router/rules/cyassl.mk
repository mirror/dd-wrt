CONFIGURE_ARGS += \
        --without-zlib \
        --enable-singleThreaded

cyassl-configure: 
	cd cyassl && ./configure $(CONFIGURE_ARGS) CFLAGS="-fPIC -DNEED_PRINTF $(COPTS)"

cyassl: cyassl-configure
	make -C cyassl

cyassl-install:
	install -D cyassl/src/.libs/libcyassl.so.0.0.0 $(INSTALLDIR)/cyassl/usr/lib/libcyassl.so.0.0.0
	cd $(INSTALLDIR)/cyassl/usr/lib ; ln -s libcyassl.so.0.0.0 libcyassl.so.0 ; true
	cd $(INSTALLDIR)/cyassl/usr/lib ; ln -s libcyassl.so.0.0.0 libcyassl.so  ; true

cyassl-clean:
	make -C cyassl clean



