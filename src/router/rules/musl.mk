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

