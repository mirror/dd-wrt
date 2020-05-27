KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/drivers/ethernet
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

all:
	@$(MAKE) -C $(KDIR) M=$(PWD)/sk98lin modules

install:
	install -v -m 644 -D sk98lin/sk98lin.ko $(IDIR)/sk98lin.ko

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD)/sk98lin clean
