ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/init.d
	install -d $(INSTALLDIR)/usr/local
	install -d $(INSTALLDIR)/sbin
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/lib

	install services $(INSTALLDIR)/etc
ifdef CONFIG_SER
	install profile2 $(INSTALLDIR)/etc/profile
else
	install profile $(INSTALLDIR)/etc
endif
ifeq ($(CONFIG_MSSID),y)
	install motd $(INSTALLDIR)/etc
else
	install motd.v23 $(INSTALLDIR)/etc/motd
endif
	install fstab $(INSTALLDIR)/etc
	install ethertypes $(INSTALLDIR)/etc
	install protocols $(INSTALLDIR)/etc
	install txackset.sh $(INSTALLDIR)/etc
	install network.overrides $(INSTALLDIR)/etc
	install init.d/rcS $(INSTALLDIR)/etc/init.d
#install nvram/nvram $(INSTALLDIR)/etc/nvram
	ln -sf /tmp/resolv.conf $(INSTALLDIR)/etc/resolv.conf
	ln -sf /tmp/hosts $(INSTALLDIR)/etc/hosts
	ln -sf /tmp/cron.d $(INSTALLDIR)/etc/cron.d
	ln -sf /tmp/etc/passwd $(INSTALLDIR)/etc/passwd
	ln -sf /tmp/etc/group $(INSTALLDIR)/etc/group
	ln -sf /usr/local/nvram $(INSTALLDIR)/etc/nvram

clean:

