
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections 

MODULE:=conntrack.startup

include $(TOP)/busybox/.config

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
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
ifeq ($(CONFIG_SENDMAIL),y)
	cp notifier.websecurity $(INSTALLDIR)/etc/config
	cp notifier.nvramconfig $(INSTALLDIR)/etc/config
	cp notifier $(INSTALLDIR)/usr/sbin
endif
endif
endif