musl-configure:
	cd musl && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS)"
	make -C musl

musl-clean:
	make -C musl clean

musl:
	make -C musl

musl-install:
	@true

