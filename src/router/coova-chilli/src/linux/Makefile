KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/
KDIR := $(LINUXDIR)
PWD := $(shell pwd)
LDFLAGS:=

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules MODPROBE=true KBUILD_MODPOST_WARN=1  $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666")

install:
	install -v -m 644 -D xt_coova.so $(IDIR)/xt_coova.so

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
