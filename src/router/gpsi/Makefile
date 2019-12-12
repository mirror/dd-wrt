
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=gpsi.startup gpsi.webservices gpsi.nvramconfig

clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)* $(INSTALLDIR)/etc/config
ifdef CONFIG_TMK
	touch $(INSTALLDIR)/etc/config/gpsi_startraw
endif
ifdef CONFIG_BKM
	touch $(INSTALLDIR)/etc/config/gpsi_startraw
endif
ifdef CONFIG_ANTAIRA
	touch $(INSTALLDIR)/etc/config/gpsi_startraw
endif
	#mkdir -p $(INSTALLDIR)/usr/sbin
	#cp st $(INSTALLDIR)/usr/sbin/show_txpower

