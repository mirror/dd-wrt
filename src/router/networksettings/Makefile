
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
ifeq ($(CONFIG_X86),y)
MODULE:=networksettings.rb532
endif
ifeq ($(CONFIG_MAGICBOX),y)
MODULE:=networksettings.rb532
endif

clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config

