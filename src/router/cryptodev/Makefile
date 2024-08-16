KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666" CFLAGS_MODULE="-DCAKE_VERSION=\\\"$(VERSION)\\\"")

install:
	install -v -m 644 -D cryptodev.ko $(IDIR)/sch_cake.ko

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
