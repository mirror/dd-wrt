lzo-clean:
	make -C lzo clean

lzo-configure:
	cd lzo && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(LTO) $(MIPS16_OPT) $(LTOFIXUP)" AR_FLAGS="cru $(LTOPLUGIN)" RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	make -C lzo clean

lzo:
	make -C lzo

lzo-install:
	@true