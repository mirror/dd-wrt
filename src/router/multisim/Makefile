
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif



all: 


install:
	
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config 
	cp multisim.* $(INSTALLDIR)/etc/config