include $(TOP)/.config

KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/netfilter
KDIR := $(LINUXDIR)
PWD := $(shell pwd)


all:
ifeq ($(CONFIG_IPV6),y)
	-@$(MAKE) -C $(KDIR) M=$(PWD) modules EXTRA_CFLAGS="-DWITH_IPV6=1" $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666") MODPROBE=true KBUILD_MODPOST_WARN=1
	-mv xt_psd.ko xt_psd_ipv6.ko
	-mv xt_TARPIT.ko xt_TARPIT_ipv6.ko
	-mv xt_geoip.ko xt_geoip_ipv6.ko
	-mv xt_lscan.ko xt_lscan_ipv6.ko
endif
	@$(MAKE) -C $(KDIR) M=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x666") MODPROBE=true KBUILD_MODPOST_WARN=1

configure:
	./xt_geoip_dl
	./xt_geoip_build -D geoip
	gcc -o resort resort.c
	./resort 4 geoip/4*
	./resort 16 geoip/6*

install:
	install -v -m 644 -D xt_lscan.ko $(IDIR)/xt_lscan.ko
	install -v -m 644 -D xt_psd.ko $(IDIR)/xt_psd.ko
	install -v -m 644 -D xt_TARPIT.ko $(IDIR)/xt_TARPIT.ko
ifeq ($(CONFIG_IPV6),y)
	-install -v -m 644 -D xt_lscan_ipv6.ko $(IDIR)/xt_lscan_ipv6.ko
	-install -v -m 644 -D xt_psd_ipv6.ko $(IDIR)/xt_psd_ipv6.ko
	-install -v -m 644 -D xt_TARPIT_ipv6.ko $(IDIR)/xt_TARPIT_ipv6.ko
endif
ifeq ($(CONFIG_GEOIP),y)
	install -v -m 644 -D xt_geoip.ko $(IDIR)/xt_geoip.ko
ifeq ($(CONFIG_IPV6),y)
	-install -v -m 644 -D xt_geoip_ipv6.ko $(IDIR)/xt_geoip_ipv6.ko
endif
	mkdir -p $(INSTALLDIR)/usr/share/xt_geoip
	install -v -m 644 -D geoip/* $(INSTALLDIR)/usr/share/xt_geoip/
endif

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f resort
