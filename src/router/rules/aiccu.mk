aiccu-configure:
	@true

aiccu:
	make EXTRA_CFLAGS="-D__UCLIBC__ -DNEED_PRINTF $(COPTS) $(MIPS16_OPT) $(LTO) -D_GNU_SOURCE" OS_NAME="Linux" OS_VERSION="dd-wrt" -C aiccu all

aiccu-clean:
	make -C aiccu clean

aiccu-install:
	install -D aiccu/unix-console/aiccu $(INSTALLDIR)/aiccu/usr/sbin/aiccu
