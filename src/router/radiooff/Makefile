
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif
ifeq ($(CONFIG_BUFFALO),y)
ifeq ($(CONFIG_AOSS),y)
MODULE:=radiooff_buffalo_aoss
else
MODULE:=radiooff_buffalo
endif
else
MODULE:=radiooff
endif


clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE).nvramconfig $(INSTALLDIR)/etc/config
	cp $(MODULE).webservices $(INSTALLDIR)/etc/config

