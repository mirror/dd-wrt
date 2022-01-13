lsof-configure:
	cd lsof && \
		LINUX_CLIB="-DGLIBCV=2" \
		LSOF_CC="$(CC)" \
		LSOF_VSTR="$(KERNELRELEASE)" \
		LSOF_CFGF="$(COPTS) $(MIPS16_OPT) $(LTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF -D_GNU_SOURCE -I$(TOP)/lsof/librpc" \
		LSOF_CFGL="$(COPTS) $(MIPS16_OPT) $(LTO) $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(TOP)/lsof/librpc/librpc.a" \
		LSOF_AR="$(CROSS_COMPILE)gcc-ar cr $(LTOPLUGIN)" \
		LSOF_RANLIB="$(CROSS_COMPILE)gcc-ranlib $(LTOPLUGIN)" \
		./Configure -n linux

#	cd lsof && ./configure --prefix=/usr --libdir=/usr/lib --host=$(ARCH)-linux CC="$(CC)" CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF"

lsof:
	$(MAKE) -C lsof all

lsof-clean:
	if test -e "lsof/Makefile"; then make -C lsof clean; fi
	@true

lsof-install:
	install -D lsof/lsof $(INSTALLDIR)/lsof/usr/sbin/lsof
#