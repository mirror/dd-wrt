
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=usb
MODULE_ADV:=adv_usb

all:
ifeq ($(CONFIG_USB_ADVANCED),y)
	cp $(MODULE_ADV)* $(TOP)/httpd/ej_temp/
else
	cp $(MODULE)* $(TOP)/httpd/ej_temp/
endif

clean:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
ifeq ($(CONFIG_USB_ADVANCED),y)
	cp $(MODULE_ADV)* $(INSTALLDIR)/etc/config
	cp usb.umount $(INSTALLDIR)/etc/config
else
	cp $(MODULE)* $(INSTALLDIR)/etc/config
endif
