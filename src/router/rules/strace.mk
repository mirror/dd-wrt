ifeq ($(ARCH),armeb)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-arm
endif
ifeq ($(ARCH),arm)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-arm
endif
ifeq ($(ARCH),mips)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),mips64)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),mipsel)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-mips
endif
ifeq ($(ARCH),powerpc)
#export LIBUNWIND_OPT = --with-libunwind
#export LIBUNWIND_LIB = -lunwind-ppc32
endif
ifeq ($(ARCH),i386)
#export LIBUNWIND_LIB = -lunwind -lunwind-x86
endif
ifeq ($(ARCH),x86_64)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind -lunwind-x86_64
endif
ifeq ($(ARCH),aarch64)
export LIBUNWIND_OPT = --with-libunwind
export LIBUNWIND_LIB = -lunwind-aarch64
endif


strace-configure:
	make -C $(TOP)/libunwind
	cd strace && ./configure \
		--prefix=/usr \
		--disable-gcc-Werror \
		--libdir=/usr/lib \
		--sysconfdir=/etc \
		--enable-mpers=no \
		--host=$(ARCH)-linux \
		$(LIBUNWIND_OPT) \
		CC="$(CC)" \
		CFLAGS="-I$(TOP)/kernel_headers/$(KERNELRELEASE)/include $(LTO) $(COPTS) $(MIPS16_OPT) -fcommon -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/openssl -DNEED_PRINTF -I$(TOP)/libunwind/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections $(LDLTO) -L$(TOP)/openssl -L$(TOP)/libunwind/src/.libs $(LIBUNWIND_LIB)" \
		AR_FLAGS="cru $(LTOPLUGIN)" \
		RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

strace: libunwind
	rm -f strace/src/sen.h
	$(MAKE) -C strace

strace-clean: 
	if test -e "strace/Makefile"; then $(MAKE) -C strace clean ; fi

strace-install: 
	$(MAKE) -C strace install DESTDIR=$(INSTALLDIR)/strace
	rm -rf $(INSTALLDIR)/strace/usr/share
	