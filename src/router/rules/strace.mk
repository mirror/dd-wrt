ifeq ($(ARCH),armeb)
export LIBUNWIND_LIB = -lunwind-arm
endif
ifeq ($(ARCH),arm)
export LIBUNWIND_LIB = -lunwind-arm
endif
ifeq ($(ARCH),mips)
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),mips64)
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),mipsel)
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),powerpc)
export LIBUNWIND_LIB = -lunwind-ppc
endif
ifeq ($(ARCH),i386)
export LIBUNWIND_LIB = -lunwind-ppc
endif
ifeq ($(ARCH),x86_64)
export LIBUNWIND_LIB = -lunwind-ppc
endif
ifeq ($(ARCH),aarch64)
export LIBUNWIND_LIB = -lunwind-ppc
endif


strace-configure:
	cd strace && ./configure \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--host=$(ARCH)-linux \
		--with-libunwind \
		CC="$(CC)" \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -DNEED_PRINTF -I$(TOP)/libunwind/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -L$(TOP)/libunwind/src/.libs $(LIBUNWIND_LIB)"

strace: 
	$(MAKE) -C strace

strace-clean: 
	if test -e "strace/Makefile"; then $(MAKE) -C strace clean ; fi

strace-install: 
	$(MAKE) -C strace install DESTDIR=$(INSTALLDIR)/strace
	rm -rf $(INSTALLDIR)/strace/usr/share
	