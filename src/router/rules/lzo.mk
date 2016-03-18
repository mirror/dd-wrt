lzo-clean:
	make -j 4 -C lzo clean

lzo-configure:
	cd lzo && ./configure --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT)"
	make -j 4 -C lzo clean

lzo:
	make -j 4 -C lzo

lzo-install:
	@true