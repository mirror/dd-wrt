qrencode-configure:
	cd qrencode && ./autogen.sh
	cd qrencode && ./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT)  -ffunction-sections -fdata-sections -Wl,--gc-sections" LDFLAGS="$(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections" --prefix=/usr --without-png

qrencode:
	$(MAKE) -C qrencode

qrencode-clean:
	$(MAKE) -C qrencode clean

qrencode-install:
	install -D $(TOP)/qrencode/qrencode $(INSTALLDIR)/qrencode/usr/sbin/qrencode