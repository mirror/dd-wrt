
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MOTD_ADD:=
ifdef CONFIG_ANTAIRA
 MOTD_ADD:=.antaira
endif
ifdef CONFIG_TMK
 MOTD_ADD:=.kmt
endif
ifdef CONFIG_BKM
 MOTD_ADD:=.bkm
endif
ifdef CONFIG_TRIMAX
 MOTD_ADD:=.m2m
endif
ifdef CONFIG_SANSFIL
 MOTD_ADD:=.sansfil
endif
ifdef CONFIG_KORENRON
 MOTD_ADD:=.korenron
endif
ifdef CONFIG_TESTEM
 MOTD_ADD:=.testem
endif
ifdef CONFIG_HOBBIT
 MOTD_ADD:=.hobbit
endif
ifdef CONFIG_ONNET
ifdef CONFIG_ONNET_BLANK
 MOTD_ADD:=.onnet_blank
else
 MOTD_ADD:=.onnet
endif
endif
ifdef CONFIG_HDWIFI
 MOTD_ADD:=.hdwifi
endif
ifdef CONFIG_ESPOD
 MOTD_ADD:=.espod
endif
ifdef CONFIG_RAYTRONIK
 MOTD_ADD:=.raytronik
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
ifdef CONFIG_USB
	install tune.sh $(INSTALLDIR)/usr/sbin
endif
ifdef CONFIG_NORTHSTAR
	install -d $(INSTALLDIR)/etc/cfe
	install cfe/cfe_600.bin $(INSTALLDIR)/etc/cfe
endif
ifdef CONFIG_MT7612
	install -d $(INSTALLDIR)/etc/wlan
	install rt2880/SingleSKU.dat $(INSTALLDIR)/etc/wlan
endif
ifdef CONFIG_MT7615
	install -d $(INSTALLDIR)/etc/wlan/
	cp rt2880/mt7615/*.dat $(INSTALLDIR)/etc/wlan
	install rt2880/MT7615*.bin $(INSTALLDIR)/etc/wlan
endif
ifdef CONFIG_RT3052
	install -d $(INSTALLDIR)/etc/wlan
	install rt2880/RT30*.bin $(INSTALLDIR)/etc/wlan
	install rt2880/RT33*.bin $(INSTALLDIR)/etc/wlan
	install rt2880/RT36*.bin $(INSTALLDIR)/etc/wlan
	install rt2880/RT38*.bin $(INSTALLDIR)/etc/wlan
	install rt2880/MT76*.BIN $(INSTALLDIR)/etc/wlan
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
ifeq ($(CONFIG_DIST),"micro")
	install profile_micro $(INSTALLDIR)/etc
else
	install profile $(INSTALLDIR)/etc
endif
endif
ifdef CONFIG_JEMALLOC
	install profile.jemalloc $(INSTALLDIR)/etc/profile
endif
ifndef CONFIG_MAKSAT
ifndef CONFIG_WIKINGS
ifndef CONFIG_ERC
ifndef CONFIG_NEXTMEDIA
ifdef CONFIG_MSSID
	install motd$(MOTD_ADD) $(INSTALLDIR)/etc/motd
else
	install motd.v23 $(INSTALLDIR)/etc/motd
endif
endif
endif
endif
endif

ifdef CONFIG_XSCALE
	install convert $(INSTALLDIR)/etc
	install kendin $(INSTALLDIR)/etc
#	cp firmware/*.dat $(INSTALLDIR)/usr/lib/firmware
ifndef CONFIG_WP18
ifndef CONFIG_WG302
ifndef CONFIG_NOP8670
ifndef CONFIG_CAMBRIA
#	cp firmware/redboot.tg7 $(INSTALLDIR)/usr/lib/firmware
else
#	cp firmware/redboot_cambria.tg7 $(INSTALLDIR)/usr/lib/firmware/redboot.tg7
endif
ifdef CONFIG_WRT300NV2
#	rm -f $(INSTALLDIR)/usr/lib/firmware/*.tg7
endif
endif
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
ifdef CONFIG_HTOP
	install -d $(INSTALLDIR)/etc/terminfo
	cp -r terminfo $(INSTALLDIR)/etc/
endif
ifdef CONFIG_WDR4900
	mkdir -p $(INSTALLDIR)/lib/firmware
	ln -sf /tmp/firmware/pci_wmac0.eeprom $(INSTALLDIR)/lib/firmware/pci_wmac0.eeprom
	ln -sf /tmp/firmware/pci_wmac1.eeprom $(INSTALLDIR)/lib/firmware/pci_wmac1.eeprom
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
	ln -sf /proc/mounts $(INSTALLDIR)/etc/mtab
ifndef CONFIG_GATEWORX
	ln -sf /usr/local/nvram $(INSTALLDIR)/etc/nvram	
else
	mkdir -p $(INSTALLDIR)/etc/nvram
endif
clean:

