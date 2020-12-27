
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=networksettings.brcm
ifeq ($(CONFIG_RB500),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_XSCALE),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_DANUBE),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_AR9),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_X86),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_MAGICBOX),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_LAGUNA),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_VENTANA),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_NEWPORT),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_RB600),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_AP83),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_LSX),y)
MODULE:=networksettings.rb532
endif
#ifeq ($(CONFIG_BUFFALO),y)
#MODULE:=networksettings.buffalo
#endif

clean:
	@true
all:
	cp $(MODULE)* $(TOP)/httpd/ej_temp/
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config

