slang-configure:
	cd slang && ./configure --host=$(ARCH)-uclibc-linux CFLAGS="$(COPTS)  $(MIPS16_OPT) -I$(TOP)/zlib -L$(TOP)/zlib -L$(INSTALLDIR)/util-linux/usr/lib" --enable-shared \
		--enable-static \
		--without-png \
		--libdir=/usr/lib \
		--enable-debug=no 
	make -C slang clean
	make -C slang

slang:
	make -C slang

slang-clean
	if test -e "slang/Makefile"; then $(MAKE) -C slang clean; fi
