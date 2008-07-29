
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


MODULE:=conntrack.startup
ifeq ($(CONFIG_RB500),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_XSCALE),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_X86),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_MAGICBOX),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_FONERA),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_LS2),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_LS5),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_MERAKI),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_WHRAG108),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_PB42),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_LSX),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_DANUBE),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_TW6600),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_CA8),y)
MODULE:=conntrack.26
endif

clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config

