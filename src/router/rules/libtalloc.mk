TALLOCCROSS = " \
	\nChecking simple C program: OK \
	\nrpath library support: OK \
	\n-Wl,--version-script support: OK \
	\nChecking getconf LFS_CFLAGS: NO \
	\nChecking for large file support without additional flags: OK \
	\nChecking for -D_LARGE_FILES: OK \
	\nChecking correct behavior of strtoll: NO \
	\nChecking for working strptime: OK \
	\nChecking for C99 vsnprintf: OK \
	\nChecking for HAVE_SHARED_MMAP: OK \
	\nChecking for HAVE_MREMAP: OK \
	\nChecking for HAVE_INCOHERENT_MMAP: NO \
	\nChecking for HAVE_SECURE_MKSTEMP: OK \
	\nChecking for HAVE_IFACE_GETIFADDRS: OK \
	\nChecking for kernel change notify support: OK \
	\nChecking for Linux kernel oplocks: OK \
	\nChecking for kernel share modes: OK \
	\nChecking if can we convert from CP850 to UCS-2LE: OK \
	\nChecking if can we convert from UTF-8 to UCS-2LE: OK \
	\nChecking whether we can use Linux thread-specific credentials with 32-bit system calls: OK \
	\nChecking whether we can use Linux thread-specific credentials: OK \
	\nChecking whether setreuid is available: OK \
	\nChecking whether setresuid is available: OK \
	\nChecking whether seteuid is available: OK \
	\nChecking whether fcntl locking is available: OK \
	\nChecking for the maximum value of the 'time_t' type: OK \
	\nChecking whether the realpath function allows a NULL argument: OK \
	\nChecking whether POSIX capabilities are available: OK \
	\nChecking for ftruncate extend: OK \
	\nvfs_fileid checking for statfs() and struct statfs.f_fsid: OK \
	\ngetcwd takes a NULL argument: OK \
	\nChecking value of NSIG: \"65\" \
	\nChecking value of _NSIG: \"65\" \
	\nChecking value of SIGRTMAX: \"64\" \
	\nChecking value of SIGRTMIN: \"34\" \
	\nChecking if toolchain accepts -fstack-protector: OK \
	\n"

libtalloc-configure:
	(cd libtalloc; \
		echo -e >cache.txt $(TALLOCCROSS) " \
			\nChecking uname machine type: \"$(ARCH)\" \
			\nChecking uname release type: \"3.9.0\" \
			\nChecking uname sysname type: \"Linux\" \
			\nChecking uname version type: \"3.9.0\" \
		\n" ; LDFLAGS="$(COPTS)" CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)" PYTHONHASHSEED=1 CC="ccache $(ARCH)-linux-uclibc-gcc" \
		./buildtools/bin/waf configure \
			--prefix=/usr \
			--sysconfdir=/etc \
			--localstatedir=/var \
			--cross-compile \
			--disable-python \
			--cross-answers=$(TOP)/libtalloc/cache.txt \
			--disable-rpath \
			--disable-rpath-install \
	)


#	cd libtalloc && ./configure --hostcc=gcc --prefix=/usr CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB)"

libtalloc:
	make -C libtalloc
	
libtalloc-install:
	mkdir -p $(INSTALLDIR)/libtalloc/usr/lib
	cp libtalloc/bin/default/libtalloc.so $(INSTALLDIR)/libtalloc/usr/lib/libtalloc.so.2