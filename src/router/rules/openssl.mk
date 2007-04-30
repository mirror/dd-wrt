openssl:
	$(MAKE) -C openssl CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc 
#	$(MAKE) -C openssl CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc
	$(MAKE) -C openssl build-shared CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc
	$(MAKE) -C openssl build_apps CC="$(ARCH)-linux-uclibc-gcc -I$(TOP)/zlib" MAKEDEPPROG=$(ARCH)-linux-uclibc-gcc
	

openssl-install:
	install -D openssl/libcrypto.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.0.9.8
#	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libcrypto.so
	install -D openssl/libssl.so.0.9.8 $(INSTALLDIR)/openssl/usr/lib/libssl.so.0.9.8
	install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
#	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libssl.so
	@true

openssl-clean:
	$(MAKE) -C openssl clean

