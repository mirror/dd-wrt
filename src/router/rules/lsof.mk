lsof-configure:
	cd lsof && \
		LINUX_CLIB="-DGLIBCV=2" \
		LSOF_CC="$(CC)" \
		LSOF_VSTR="$(KERNELRELEASE)" \
		LSOF_CFGF="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -DHASNORPC_H" \
		LSOF_CFGL="-ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF" \
		LSOF_AR="$(CROSS_COMPILE)ar cr" \
		LSOF_RANLIB="$(CROSS_COMPILE)ranlib" \
		./Configure -n linux

#	cd lsof && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF"

lsof:
	$(MAKE) -C lsof

lsof-clean:
	if test -e "lsof/Makefile"; then make -C lsof clean; fi
	@true

lsof-install:
	install -D lsof/lsof $(INSTALLDIR)/lsof/usr/sbin/lsof
#