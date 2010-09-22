obj-$(CONFIG_NVRAM) += nvram
obj-$(CONFIG_WIRELESS_TOOLS) += wireless-tools
obj-$(CONFIG_SHARED) += libutils
obj-$(CONFIG_SHARED) += shared
obj-$(CONFIG_LIBNET) += libnet
obj-$(CONFIG_OPENSSL) += openssl openssl-shared openssl-apps
obj-$(CONFIG_MATRIXSSL) += matrixssl
obj-$(CONFIG_CTORRENT) += ctorrent
obj-$(CONFIG_SFTPSERVER) += sftp-server
obj-$(CONFIG_HTTPD) += httpd
obj-$(CONFIG_RC) += rc services 
obj-$(CONFIG_LIBBCM) += libbcm
obj-$(CONFIG_WWW) += www
obj-$(CONFIG_GLIBC) += lib.$(ARCH)$(ARCHEXT)
obj-$(CONFIG_UCLIBC) += lib.$(ARCH)$(ARCHEXT)
obj-$(CONFIG_BRIDGE) += bridge
obj-$(CONFIG_RFLOW) += rflow
obj-$(CONFIG_DROPBEAR_SSHD) += zlib dropbear
obj-$(CONFIG_DHCPFORWARD) += dhcpforwarder
obj-$(CONFIG_BUSYBOX) += busybox
#obj-$(CONFIG_TELNET) += telnetd
obj-$(CONFIG_DNSMASQ) += dnsmasq
obj-$(CONFIG_MOXA) += moxa
obj-$(CONFIG_IPTABLES) += iptables
obj-$(CONFIG_LIBIPT) += iptables
obj-$(CONFIG_IPSEC) += ipsec
obj-$(CONFIG_LIBPCAP) += libpcap
obj-$(CONFIG_WIVIZ) += wiviz2
obj-$(CONFIG_TCPDUMP) += tcpdump
obj-$(CONFIG_KISMETDRONE) += kismet-devel
obj-$(CONFIG_NETSTATNAT) += netstatnat
obj-$(CONFIG_SES) += ses
obj-$(CONFIG_WPA_SUPPLICANT) += wpa_supplicant
obj-$(CONFIG_HOSTAPD) += hostapd
obj-$(CONFIG_NETCONF) += netconf
obj-$(CONFIG_NTP) += ntpclient
obj-$(CONFIG_PPP) += ppp
# AhMan March 19 2005
obj-$(CONFIG_PPPOE) += pppoe
obj-$(CONFIG_UDHCPD) += udhcpd
obj-$(CONFIG_UDHCPC) += udhcpd
obj-$(CONFIG_UPNP) += upnp
ifneq ($(ARCHITECTURE),broadcom)
obj-$(CONFIG_MADWIFI) += madwifi relayd
obj-$(CONFIG_MADWIFI_MIMO) += madwifi_mimo relayd
else
obj-$(CONFIG_MADWIFI) += madwifi
obj-$(CONFIG_MADWIFI_MIMO) += madwifi_mimo
endif
obj-$(CONFIG_ETC) += etc
#obj-$(CONFIG_VLAN) += vlan
obj-$(CONFIG_IPROUTE2) += iproute2
obj-$(CONFIG_EBTABLES) += ebtables
obj-$(CONFIG_PPTPD) += pptpd
obj-$(CONFIG_PIPSEC) += pipsec
obj-$(CONFIG_FROTTLE) += frottle
obj-$(CONFIG_WOL) += wol
obj-$(CONFIG_SNMP) += snmp
obj-$(CONFIG_RADVD) += radvd
obj-$(CONFIG_SPUTNIK_APD) += sputnik

#obj-$(CONFIG_ADM6996) += adm6996
##################################################################
CONFIG_OTHERS=y
#obj-$(CONFIG_ADM6996) += adm6996
obj-$(CONFIG_L2TP) += xl2tpd

obj-$(CONFIG_CHILLISPOT) += chillispot
obj-$(CONFIG_PARPROUTED) += parprouted
obj-$(CONFIG_HEARTBEAT) += bpalogin
obj-$(CONFIG_TFTPD) += tftpd
obj-$(CONFIG_CRON) += cron
obj-$(CONFIG_PPTP) += pptp-client
obj-$(CONFIG_PPPD) += pppd
obj-$(CONFIG_ZEBRA) += zebra
obj-$(CONFIG_BIRD) += bird
obj-$(CONFIG_DDNS) += inadyn
obj-$(CONFIG_OTHERS) += others
obj-$(CONFIG_EOU) += eou
obj-$(CONFIG_OPENSER) += openser
obj-$(CONFIG_MILKFISH) += milkfish
obj-$(CONFIG_MC) += mc
obj-$(CONFIG_NOCAT) += nocat
obj-$(CONFIG_RTPPROXY) += rtpproxy
obj-$(CONFIG_SAMBA) += samba
obj-$(CONFIG_SAMBA3) += samba3
obj-$(CONFIG_NTFS3G) += ntfs-3g
obj-$(CONFIG_RADAUTH) += radauth
ifneq ($(CONFIG_FONERA),y)
ifneq ($(CONFIG_XSCALE),y)
obj-$(CONFIG_MMC) += mmc
else
obj-$(CONFIG_MMC) += mmc-ixp4xx
endif
else
obj-$(CONFIG_MMC) += mmc-fonera
endif
obj-$(CONFIG_ZEROIP) += shat
obj-$(CONFIG_KAID) += kaid
obj-$(CONFIG_ROBOCFG) += robocfg
obj-$(CONFIG_MULTICAST) += igmp-proxy
obj-$(CONFIG_SKYTRON) += skytron
obj-$(CONFIG_OPENVPN) += openvpn
obj-$(CONFIG_OLSRD) += olsrd
obj-$(CONFIG_FDISK) += fdisk
ifneq ($(CONFIG_MADWIFI),y)
ifneq ($(CONFIG_MADWIFI_MIMO),y)
obj-$(CONFIG_NAS) += nas
obj-$(CONFIG_WLCONF) += wlconf
endif
endif
obj-$(CONFIG_UTILS) += utils
obj-$(CONFIG_MTR) += mtr



obj-$(CONFIG_PCIUTILS) += pciutils
#obj-$(CONFIG_E2FSPROGS) += e2fsprogs
obj-$(CONFIG_HTTPREDIRECT) += http-redirect
obj-$(CONFIG_SMTPREDIRECT) += smtp-redirect
obj-$(CONFIG_SPUTNIK_APD) += sputnik
obj-$(CONFIG_OVERCLOCKING) += overclocking
obj-$(CONFIG_PROXYWATCHDOG) += proxywatchdog
obj-$(CONFIG_JFFS2) += jffs2
obj-$(CONFIG_LANGUAGE) += language
obj-$(CONFIG_NETWORKSETTINGS) += networksettings
obj-$(CONFIG_ROUTERSTYLE) += routerstyle
obj-$(CONFIG_SCHEDULER) += scheduler
obj-$(CONFIG_SYSLOG) += syslog
obj-$(CONFIG_TELNET) += telnet
obj-$(CONFIG_WDSWATCHDOG) += wdswatchdog
obj-$(CONFIG_IPV6) += ipv6
obj-$(CONFIG_CONNTRACK) += conntrack
obj-$(CONFIG_RADIOOFF) += radiooff
obj-$(CONFIG_PHP) += php5
obj-$(CONFIG_NCURSES) += ncurses
obj-$(CONFIG_IFTOP) += iftop
obj-$(CONFIG_IPTRAF) += iptraf
obj-$(CONFIG_WIFIDOG) += wifidog
obj-$(CONFIG_HWMON) += hwmon
#obj-$(CONFIG_RSTATS) += rstats
obj-$(CONFIG_STABRIDGE) += stabridge
obj-$(CONFIG_EOP_TUNNEL) += eop-tunnel
obj-$(CONFIG_AIRCRACK) += aircrack-ng
obj-$(CONFIG_MOXA) += moxa
obj-$(CONFIG_BONDING) += ifenslave
obj-$(CONFIG_NSTX) += nstx
obj-$(CONFIG_SQUID) += squid
obj-$(CONFIG_IPERF) += iperf
obj-$(CONFIG_NTPD) += ntpd
obj-$(CONFIG_GPSD) += gpsd
obj-$(CONFIG_PHP5) += php5
obj-$(CONFIG_FREERADIUS) += freeradius
#obj-$(CONFIG_EAD) += ead

obj-$(CONFIG_SCDP) += scdp
obj-$(CONFIG_SES) += ses
obj-$(CONFIG_PRINTER_SERVER) += ippd
obj-$(CONFIG_FTP) += proftpd
obj-$(CONFIG_PCMCIA) += pcmcia 
obj-$(CONFIG_PCMCIA) += microcom
obj-$(CONFIG_PCMCIA) += gcom
obj-$(CONFIG_PCMCIA) += nvtlstatus 
obj-$(CONFIG_PCMCIA) += setserial
obj-$(CONFIG_COMGT) += comgt
obj-$(CONFIG_MEDIASERVER) += mediaserver
obj-$(CONFIG_QUAGGA) += quagga
obj-$(CONFIG_VPNC) += vpnc
obj-$(CONFIG_STUCK) += stuck_beacon
obj-$(CONFIG_GPSI) += gpsi
obj-$(CONFIG_BMON) += bmon
obj-$(CONFIG_SERCD) += sercd
obj-$(CONFIG_P910ND) += p910nd
obj-$(CONFIG_HOSTAPD2) += hostapd2
obj-$(CONFIG_WPA_SUPPLICANT2) += wpa_supplicant2
obj-$(CONFIG_MIITOOL) += net-tools
obj-$(CONFIG_TOR) += tor
obj-$(CONFIG_RSTP) += rstp
obj-$(CONFIG_STRACE) += strace
obj-$(CONFIG_OPENLLDP) += openlldp
obj-$(CONFIG_WGETS) += wgets
obj-$(CONFIG_USB) += usb disktype
obj-$(CONFIG_ASTERISK) += asterisk
obj-$(CONFIG_ZAPTEL) += zaptel
obj-$(CONFIG_WAVESAT) += wavesat
obj-$(CONFIG_POUND) += pound
obj-$(CONFIG_VNCREPEATER) += vncrepeater
obj-$(CONFIG_SWCONFIG) += swconfig
obj-$(CONFIG_NPROBE) += nprobe
obj-$(CONFIG_MTR) += mtr
obj-$(CONFIG_SNOOP) += snoop
obj-$(CONFIG_AOSS) += aoss
obj-$(CONFIG_AP_SERV) += ap-serv
obj-$(CONFIG_BUFFALO) += buffalo_flash
#obj-$(CONFIG_RELAYD) += relayd
#obj-$(CONFIG_ATH9K) += ath9k iw crda libnltiny
obj-$(CONFIG_LIBNLTINY) += libnltiny
obj-$(CONFIG_HOTPLUG2) += hotplug2 udev
obj-$(CONFIG_UBOOTENV) += ubootenv

obj-y+=anchorfree
obj-y+=ttraff
obj-y+=speedtest
obj-$(CONFIG_MKIMAGE) += mkimage
obj-y+=configs

obj-configure := $(foreach obj,$(obj-y) $(obj-n),$(obj)-configure)
obj-checkout := $(foreach obj,$(obj-y) $(obj-n),$(obj)-checkout)
obj-update := $(foreach obj,$(obj-y) $(obj-n),$(obj)-update)

configs-checkout:
	rm -rf $(TOP)/kromo/dd-wrt/34telecom
	rm -rf $(TOP)/kromo/dd-wrt/3com
	rm -rf $(TOP)/kromo/dd-wrt/alfa
	rm -rf $(TOP)/kromo/dd-wrt/allnet
	rm -rf $(TOP)/kromo/dd-wrt/buffalo
	rm -rf $(TOP)/kromo/dd-wrt/cesar
	rm -rf $(TOP)/kromo/dd-wrt/cnc
	rm -rf $(TOP)/kromo/dd-wrt/corenet
	rm -rf $(TOP)/kromo/dd-wrt/ddlan
	rm -rf $(TOP)/kromo/dd-wrt/erc
	rm -rf $(TOP)/kromo/dd-wrt/ggew
	rm -rf $(TOP)/kromo/dd-wrt/glauco
	rm -rf $(TOP)/kromo/dd-wrt/kmt
	rm -rf $(TOP)/kromo/dd-wrt/kodata
	rm -rf $(TOP)/kromo/dd-wrt/maksat
	rm -rf $(TOP)/kromo/dd-wrt/maksat_coral
	rm -rf $(TOP)/kromo/dd-wrt/nmn
	rm -rf $(TOP)/kromo/dd-wrt/powernoc
	rm -rf $(TOP)/kromo/dd-wrt/telcom
	rm -rf $(TOP)/kromo/dd-wrt/thom
	rm -rf $(TOP)/kromo/dd-wrt/trimax
	rm -rf $(TOP)/kromo/dd-wrt/wts
	rm -rf $(TOP)/kromo/dd-wrt/mynetway
	rm -rf $(TOP)/kromo/dd-wrt/wikings
	svn co svn://svn.dd-wrt.com/private/designs/34telecom $(TOP)/kromo/dd-wrt/34telecom
	svn co svn://svn.dd-wrt.com/private/designs/3com $(TOP)/kromo/dd-wrt/3com
	svn co svn://svn.dd-wrt.com/private/designs/alfa $(TOP)/kromo/dd-wrt/alfa
	svn co svn://svn.dd-wrt.com/private/designs/allnet $(TOP)/kromo/dd-wrt/allnet
	svn co svn://svn.dd-wrt.com/private/designs/buffalo $(TOP)/kromo/dd-wrt/buffalo
	svn co svn://svn.dd-wrt.com/private/designs/cesar $(TOP)/kromo/dd-wrt/cesar
	svn co svn://svn.dd-wrt.com/private/designs/cnc $(TOP)/kromo/dd-wrt/cnc
	svn co svn://svn.dd-wrt.com/private/designs/corenet $(TOP)/kromo/dd-wrt/corenet
	svn co svn://svn.dd-wrt.com/private/designs/ddlan $(TOP)/kromo/dd-wrt/ddlan
	svn co svn://svn.dd-wrt.com/private/designs/erc $(TOP)/kromo/dd-wrt/erc
	svn co svn://svn.dd-wrt.com/private/designs/ggew $(TOP)/kromo/dd-wrt/ggew
	svn co svn://svn.dd-wrt.com/private/designs/glauco $(TOP)/kromo/dd-wrt/glauco
	svn co svn://svn.dd-wrt.com/private/designs/kmt $(TOP)/kromo/dd-wrt/kmt
	svn co svn://svn.dd-wrt.com/private/designs/kodata $(TOP)/kromo/dd-wrt/kodata
	svn co svn://svn.dd-wrt.com/private/designs/maksat $(TOP)/kromo/dd-wrt/maksat
	svn co svn://svn.dd-wrt.com/private/designs/maksat_coral $(TOP)/kromo/dd-wrt/maksat_coral
	svn co svn://svn.dd-wrt.com/private/designs/nmn $(TOP)/kromo/dd-wrt/nmn
	svn co svn://svn.dd-wrt.com/private/designs/powernoc $(TOP)/kromo/dd-wrt/powernoc
	svn co svn://svn.dd-wrt.com/private/designs/telcom $(TOP)/kromo/dd-wrt/telcom
	svn co svn://svn.dd-wrt.com/private/designs/thom $(TOP)/kromo/dd-wrt/thom
	svn co svn://svn.dd-wrt.com/private/designs/trimax $(TOP)/kromo/dd-wrt/trimax
	svn co svn://svn.dd-wrt.com/private/designs/wts $(TOP)/kromo/dd-wrt/wts
	svn co svn://svn.dd-wrt.com/private/designs/mynetway $(TOP)/kromo/dd-wrt/mynetway
	svn co svn://svn.dd-wrt.com/private/designs/wikings $(TOP)/kromo/dd-wrt/wikings
	svn co svn://svn.dd-wrt.com/private/designs/nextmedia $(TOP)/kromo/dd-wrt/nextmedia
		

configs-update:
	svn update $(TOP)/kromo/dd-wrt/34telecom
	svn update $(TOP)/kromo/dd-wrt/3com
	svn update $(TOP)/kromo/dd-wrt/alfa
	svn update $(TOP)/kromo/dd-wrt/allnet
	svn update $(TOP)/kromo/dd-wrt/buffalo
	svn update $(TOP)/kromo/dd-wrt/cesar
	svn update $(TOP)/kromo/dd-wrt/cnc
	svn update $(TOP)/kromo/dd-wrt/corenet
	svn update $(TOP)/kromo/dd-wrt/ddlan
	svn update $(TOP)/kromo/dd-wrt/erc
	svn update $(TOP)/kromo/dd-wrt/ggew
	svn update $(TOP)/kromo/dd-wrt/glauco
	svn update $(TOP)/kromo/dd-wrt/kmt
	svn update $(TOP)/kromo/dd-wrt/kodata
	svn update $(TOP)/kromo/dd-wrt/maksat
	svn update $(TOP)/kromo/dd-wrt/maksat_coral
	svn update $(TOP)/kromo/dd-wrt/nmn
	svn update $(TOP)/kromo/dd-wrt/powernoc
	svn update $(TOP)/kromo/dd-wrt/telcom
	svn update $(TOP)/kromo/dd-wrt/thom
	svn update $(TOP)/kromo/dd-wrt/trimax
	svn update $(TOP)/kromo/dd-wrt/wts
	svn update $(TOP)/kromo/dd-wrt/mynetway
	svn update $(TOP)/kromo/dd-wrt/wikings
	svn update $(TOP)/kromo/dd-wrt/nextmedia

configs-clean:
	@true

configs-install:
	@true

configs:
	@true
