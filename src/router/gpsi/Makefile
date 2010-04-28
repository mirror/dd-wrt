
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=gpsi.startup

clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp st $(INSTALLDIR)/usr/sbin/show_txpower

