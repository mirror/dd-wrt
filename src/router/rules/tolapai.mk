tolapai: 
	if test -e "tolapai/Makefile"; then make -C tolapai KERNEL_SOURCE_ROOT=$(LINUXDIR) CC=$(ARCH)-linux-gcc; fi
	@true
tolapai-clean:
	if test -e "tolapai/Makefile"; then make -C tolapai Accel_clean; fi
	@true

tolapai-install:
	@true
