
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=eop-tunnel

all:
	make -C mtik

clean:
	make -C mtik clean

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	install -d $(INSTALLDIR)/usr/sbin
	cp $(MODULE)*.nvramconfig $(INSTALLDIR)/etc/config
	cp $(MODULE)*.startup $(INSTALLDIR)/etc/config
	cp $(MODULE)*.firewall $(INSTALLDIR)/etc/config
	cp mtik/eoip $(INSTALLDIR)/usr/sbin

www-install:
	install -d $(INSTALLDIR)/../www
	install -d $(INSTALLDIR)/../www/www
	cp $(MODULE)*.asp $(INSTALLDIR)/../www/www


