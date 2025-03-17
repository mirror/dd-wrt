musl-configure:
	-make -C musl clean
	cd musl && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DCRYPT_SIZE_HACK" --enable-optimize=size --disable-gcc-wrapper
	make -C musl

musl-clean:
	make -C musl clean

musl:
	make -C musl

musl-install:
	@true




musl-mimalloc-configure:
	-make -C musl-mimalloc clean
	cd musl-mimalloc && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) -DCRYPT_SIZE_HACK" --enable-optimize=size --disable-gcc-wrapper --with-malloc=external EXTRA_OBJ=$(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o
	make -C musl-mimalloc EXTRA_OBJ=$(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o

musl-mimalloc-clean:
	make -C musl-mimalloc clean

musl-mimalloc:
	rm -f $(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o
	make -C musl-mimalloc EXTRA_OBJ=$(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o $(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o

musl-mimalloc-install:
	@true

