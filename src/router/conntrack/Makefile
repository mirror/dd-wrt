
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/

MODULE:=conntrack.startup
ifeq ($(CONFIG_RB500),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_BCMMODERN),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_XSCALE),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_X86),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_RT2880),y)
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
ifeq ($(CONFIG_STORM),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_ADM5120),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_TW6600),y)
MODULE:=conntrack.26
endif
ifeq ($(CONFIG_CA8),y)
MODULE:=conntrack.26
endif


notifier: notifier.o
	$(CC) -o $@ $^ $(CFLAGS) -L$(TOP)/nvram -lnvram

clean:
	rm -f notifier notifier.o

all: notifier


install:
	
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	install -d $(INSTALLDIR)/usr/sbin
	cp $(MODULE)* $(INSTALLDIR)/etc/config
ifeq ($(CONFIG_XSCALE),y)
	cp notifier.websecurity $(INSTALLDIR)/etc/config
	cp notifier.nvramconfig $(INSTALLDIR)/etc/config
	cp notifier $(INSTALLDIR)/usr/sbin
endif
ifeq ($(CONFIG_LSX),y)
	cp notifier.websecurity $(INSTALLDIR)/etc/config
	cp notifier.nvramconfig $(INSTALLDIR)/etc/config
	cp notifier $(INSTALLDIR)/usr/sbin
endif