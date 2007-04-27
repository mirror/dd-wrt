matrixssl:
	$(MAKE) -C matrixssl/src all

matrixssl-install:
	echo "nischt"
#	install -D matrixssl/src/libmatrixssl.so $(INSTALLDIR)/matrixssl/usr/lib/libmatrixssl.so
#	$(STRIP) $(INSTALLDIR)/matrixssl/usr/lib/libmatrixssl.so

matrixssl-clean:
	$(MAKE) -C matrixssl/src clean


