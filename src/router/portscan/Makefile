include $(TOP)/.config

KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/netfilter
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

configure:
	./xt_geoip_dl
	./xt_geoip_build -D geoip

all:
	@$(MAKE) -C $(KDIR) M=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666")

install:
	install -v -m 644 -D xt_lscan.ko $(IDIR)/xt_lscan.ko
	install -v -m 644 -D xt_psd.ko $(IDIR)/xt_psd.ko
	install -v -m 644 -D xt_TARPIT.ko $(IDIR)/xt_TARPIT.ko
ifeq ($(CONFIG_GEOIP),y)
	install -v -m 644 -D xt_geoip.ko $(IDIR)/xt_geoip.ko
	mkdir -p $(INSTALLDIR)/usr/share/xt_geoip
	install -v -m 644 -D geoip/* $(INSTALLDIR)/usr/share/xt_geoip/
endif

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
