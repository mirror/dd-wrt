KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	install -v -m 644 -D cryptodev.ko $(IDIR)/cryptodev.ko

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
