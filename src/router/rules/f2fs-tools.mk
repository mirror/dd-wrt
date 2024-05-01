f2fs-tools-configure: util-linux
	cd f2fs-tools && ./autogen.sh 
	cd f2fs-tools && ./configure \
	    --prefix=/usr \
	    --without-selinux \
	    --host=$(ARCH)-linux \
	    CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -I$(TOP)/util-linux/libuuid/src -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DNEED_PRINTF" \
	    LDFLAGS="-L$(TOP)/util-linux/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS)"

f2fs-tools:
	make -C f2fs-tools DEBUG= Q= 


f2fs-tools-clean:
	make -C f2fs-tools DEBUG= Q= clean

f2fs-tools-install:
	make -C f2fs-tools install DESTDIR=$(INSTALLDIR)/f2fs-tools
	rm -rf $(INSTALLDIR)/f2fs-tools/usr/share 
	-rm -f $(INSTALLDIR)/f2fs-tools/usr/lib/*.a
	-rm -f $(INSTALLDIR)/f2fs-tools/usr/lib/*.la
