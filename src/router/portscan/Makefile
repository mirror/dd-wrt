KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/netfilter
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666")

install:
	install -v -m 644 -D xt_lscan.ko $(IDIR)/xt_lscan.ko
	install -v -m 644 -D xt_psd.ko $(IDIR)/xt_psd.ko
	install -v -m 644 -D xt_TARPIT.ko $(IDIR)/xt_TARPIT.ko

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
