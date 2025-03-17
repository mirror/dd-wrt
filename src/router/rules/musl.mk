ifeq ($(ARCH),arm)
MIMALLOC_OPT=-Os
endif
ifeq ($(ARCH),aarch64)
MIMALLOC_OPT=-O3
endif
ifeq ($(ARCH),armeb)
MIMALLOC_OPT=-Os
endif
ifeq ($(ARCH),i386)
MIMALLOC_OPT=-O3
endif
ifeq ($(ARCH),x86_64)
MIMALLOC_OPT=-O3
endif
ifeq ($(ARCH),mips)
MIMALLOC_OPT=-Os
endif
ifeq ($(ARCH),mips64)
MIMALLOC_OPT=-O3
endif
ifeq ($(ARCH),mipsel)
MIMALLOC_OPT=-Os
endif
ifeq ($(ARCH),powerpc)
MIMALLOC_OPT=-Os
endif



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

	make -C musl-mimalloc EXTRA_OBJ=$(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o MIMALLOC_OPT=$(MIMALLOC_OPT) $(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o
	make -C musl-mimalloc EXTRA_OBJ=$(TOP)/musl-mimalloc/src/malloc/external/mimalloc.o

musl-mimalloc-install:
	@true

