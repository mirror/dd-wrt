
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=overclocking

all:
	cp $(MODULE)* $(TOP)/httpd/ej_temp

clean:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config

