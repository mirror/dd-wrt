EXTRA_KCONFIG:= \
	CONFIG_I2C_GPIO_CUSTOM=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" \
	O=$(LINUXDIR) \
	SUBDIRS="$(TOP)/i2c-gpio-custom/src" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

all:
	$(MAKE) -C "$(LINUXDIR)" \
		$(MAKE_OPTS) \
		modules

clean:
	$(MAKE) -C "$(LINUXDIR)" \
		$(MAKE_OPTS) \
		clean

install:
	$(MAKE) -C "$(LINUXDIR)" \
		$(MAKE_OPTS) \
		INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR= modules_install
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules.*

