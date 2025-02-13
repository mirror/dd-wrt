
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

#ifeq ($(CONFIG_IPSET),y)
#CFLAGS_EXTRA += -DHAVE_IPSET
#endif

MODULE:=eop-tunnel

all: install
	-make -C mtik

clean:
	-make -C mtik clean

install: all
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/usr/bin
	cp $(MODULE)*.prewall $(INSTALLDIR)/etc/config
	# cp $(MODULE)*.firewall $(INSTALLDIR)/etc/config
	cp $(MODULE)*.sh $(INSTALLDIR)/usr/bin
ifeq ($(CONFIG_IPSET),y)
	-install -d $(INSTALLDIR)/etc/config
	install -c K*.sh $(INSTALLDIR)/etc/config
	chmod 700 $(INSTALLDIR)/etc/config/K*.sh
endif
	chmod 700 $(INSTALLDIR)/usr/bin/$(MODULE)*.sh
	# chmod 700 $(INSTALLDIR)/etc/config/$(MODULE)*.firewall
	chmod 700 $(INSTALLDIR)/etc/config/$(MODULE)*.prewall
	
	-cp mtik/eoip $(INSTALLDIR)/usr/sbin
	-cp makewgkey $(INSTALLDIR)/usr/sbin
	-cp makewgpsk $(INSTALLDIR)/usr/sbin
	-cp makewgclient $(INSTALLDIR)/usr/sbin
	-cp makewgconfig $(INSTALLDIR)/usr/sbin

www-install:
	install -d $(INSTALLDIR)/../www
	install -d $(INSTALLDIR)/../www/www
	cp $(MODULE)*.asp $(INSTALLDIR)/../www/www

