libbpf: elfutils
	make -C libbpf/src \
		CFLAGS="$(COPTS) -I$(TOP)/elfutils/libelf -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/zlib" \
		LDFLAGS="-L$(TOP)/zlib -L$(TOP)/elfutils/libelf" 

libbpf-install:
	mkdir -p $(INSTALLDIR)/libbpf/usr/lib
	cp -avf libbpf/src/libbpf.so* $(INSTALLDIR)/libbpf/usr/lib