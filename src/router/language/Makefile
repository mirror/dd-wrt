
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=language

all:
	cp $(MODULE)* $(TOP)/httpd/ej_temp

clean:
	@true

install:
	cp $(MODULE)* $(TOP)/httpd/ej_temp
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config

