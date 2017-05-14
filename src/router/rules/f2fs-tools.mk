f2fs-tools-configure:
	cd f2fs-tools && ./autogen.sh 
	cd f2fs-tools && ./configure --prefix=/usr --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT)  -I$(TOP)/e2fsprogs/lib -ffunction-sections -fdata-sections -Wl,--gc-sections  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DNEED_PRINTF" LDFLAGS="-L$(TOP)/e2fsprogs/lib/uuid -ffunction-sections -fdata-sections -Wl,--gc-sections" CC="$(CC) $(COPTS)"

f2fs-tools:
	make -C f2fs-tools DEBUG= Q= 


f2fs-tools-clean:
	make -C f2fs-tools DEBUG= Q= clean

f2fs-tools-install:
	make -C f2fs-tools install DESTDIR=$(INSTALLDIR)/f2fs-tools
	rm -rf $(INSTALLDIR)/f2fs-tools/usr/share 
	-rm -f $(INSTALLDIR)/f2fs-tools/usr/lib/*.a
	-rm -f $(INSTALLDIR)/f2fs-tools/usr/lib/*.la
