
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=eop-tunnel

clean:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	cp $(MODULE)*.nvramconfig $(INSTALLDIR)/etc/config
	cp $(MODULE)*.startup $(INSTALLDIR)/etc/config
	cp $(MODULE)*.firewall $(INSTALLDIR)/etc/config

www-install:
	install -d $(INSTALLDIR)/../www
	install -d $(INSTALLDIR)/../www/www
	cp $(MODULE)*.asp $(INSTALLDIR)/../www/www

all: install
	@true

