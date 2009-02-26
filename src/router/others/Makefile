
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/init.d
	install -d $(INSTALLDIR)/usr/local
	install -d $(INSTALLDIR)/sbin
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/usr/lib
	install -d $(INSTALLDIR)/usr/lib/firmware	
	install -d $(INSTALLDIR)/lib
	install services $(INSTALLDIR)/etc
ifdef CONFIG_RT3052
	install -d $(INSTALLDIR)/etc/wlan
	install rt2880/RT30*.bin $(INSTALLDIR)/etc/wlan
else
ifdef CONFIG_RT2880
	install -d $(INSTALLDIR)/etc/wlan
	install rt2880/RT28*.bin $(INSTALLDIR)/etc/wlan
endif
endif
ifdef CONFIG_X86
	install -d $(INSTALLDIR)/boot
endif
ifdef CONFIG_SER
	install profile2 $(INSTALLDIR)/etc/profile
else
	install profile $(INSTALLDIR)/etc
endif
ifndef CONFIG_MAKSAT
ifdef CONFIG_MSSID
	install motd $(INSTALLDIR)/etc
else
	install motd.v23 $(INSTALLDIR)/etc/motd
endif
endif

ifdef CONFIG_XSCALE
	install convert $(INSTALLDIR)/etc
	install kendin $(INSTALLDIR)/etc
	cp firmware/*.dat $(INSTALLDIR)/usr/lib/firmware
ifndef CONFIG_NOP8670
ifndef CONFIG_CAMBRIA
	cp firmware/redboot.tg7 $(INSTALLDIR)/usr/lib/firmware
else
	cp firmware/redboot_cambria.tg7 $(INSTALLDIR)/usr/lib/firmware/redboot.tg7
endif
ifdef CONFIG_WRT300NV2
	rm -f $(INSTALLDIR)/usr/lib/firmware/*.tg7
endif
endif
endif
ifdef CONFIG_SNMP
ifndef CONFIG_MADWIFI
ifndef CONFIG_RT2880
	install wl_snmpd.sh $(INSTALLDIR)/etc/wl_snmpd.sh
else
	install wl_snmpd_rt2880.sh $(INSTALLDIR)/etc/wl_snmpd.sh
endif
else
	install wl_snmpd_ath.sh $(INSTALLDIR)/etc/wl_snmpd.sh
endif
endif

ifdef CONFIG_NCURSES
	install -d $(INSTALLDIR)/etc/terminfo
	cp -r terminfo $(INSTALLDIR)/etc/
endif


	install fstab $(INSTALLDIR)/etc
	install ethertypes $(INSTALLDIR)/etc
	install protocols $(INSTALLDIR)/etc
ifndef CONFIG_MSSID
	install txackset.sh $(INSTALLDIR)/etc
else
#	install patchtable.bin $(INSTALLDIR)/etc
endif
	install init.d/rcS $(INSTALLDIR)/etc/init.d
	install init.d/S01dummy $(INSTALLDIR)/etc/init.d
#install nvram/nvram $(INSTALLDIR)/etc/nvram
	ln -sf /tmp/resolv.conf $(INSTALLDIR)/etc/resolv.conf
	ln -sf /tmp/hosts $(INSTALLDIR)/etc/hosts
	ln -sf /tmp/cron.d $(INSTALLDIR)/etc/cron.d
	ln -sf /tmp/etc/passwd $(INSTALLDIR)/etc/passwd
	ln -sf /tmp/etc/group $(INSTALLDIR)/etc/group
ifndef CONFIG_GATEWORX
	ln -sf /usr/local/nvram $(INSTALLDIR)/etc/nvram	
else
	mkdir -p $(INSTALLDIR)/etc/nvram
endif
clean:

