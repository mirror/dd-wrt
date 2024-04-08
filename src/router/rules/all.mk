fixkc: $(LINUXDIR)/.config
	cp $(LINUXDIR)/.config /tmp/config.$$$$ ; \
  if [ "$(CONFIG_IPSEC_TUNNEL)" = "y" ] ; then \
    sed -e '/^CONFIG_IPSEC_TUNNEL=.*$$/d' \
				-e '/^CONFIG_CRYPTO=.*$$/d' \
        -e '/^CONFIG_CIPHERS=.*$$/d' \
        -e '/^CONFIG_DIGESTS=.*$$/d' \
        -e '/^CONFIG_CIPHER_DES=.*$$/d' \
        -e '/^CONFIG_CIPHER_3DES=.*$$/d' \
        -e '/^CONFIG_CIPHER_AES=.*$$/d' \
        -e '/^CONFIG_DIGEST_MD5=.*$$/d' \
        -e '/^CONFIG_DIGEST_SHA1=.*$$/d' \
        /tmp/config.$$$$ > /tmp/config.n.$$$$ ; \
    mv -f /tmp/config.n.$$$$ /tmp/config.$$$$ ; \
    echo 'CONFIG_IPSEC_TUNNEL=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_CRYPTO=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_DIGESTS=y' >> /tmp/config.$$$$ ; \
    echo 'CONFIG_CIPHERS=y' >> /tmp/config.$$$$ ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_MD5)" = "y" ] ; then \
      echo 'CONFIG_DIGEST_MD5=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_SHA1)" = "y" ] ; then \
      echo 'CONFIG_DIGEST_SHA1=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_DES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_DES=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_3DES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_3DES=y' >> /tmp/config.$$$$ ; \
    fi ; \
    if [ "$(CONFIG_IPSEC_TUNNEL_AES)" = "y" ] ; then \
      echo 'CONFIG_CIPHER_AES=y' >> /tmp/config.$$$$ ; \
    fi ; \
  fi && mv /tmp/config.$$$$ $(LINUXDIR)/.config


conf mconf:
	$(MAKE) -C config
	@./config/$@ ./config/Config
	$(MAKE) fixkc
        # Also configure kernel
	$(MAKE) k$@

oldconf: .config
	$(MAKE) -C config
	@./config/conf -o ./config/Config
	$(MAKE) fixkc
        # Also configure kernel
	$(MAKE) k$@

kconf:
	$(MAKE) -C $(LINUXDIR) config

kmconf:
	$(MAKE) -C $(LINUXDIR) menuconfig

koldconf: $(LINUXDIR)/.config
	$(MAKE) -C $(LINUXDIR) oldconfig

# Convenience
config: conf

menuconfig: mconf

oldconfig: oldconf


configure: $(obj-configure)

checkout: $(obj-checkout)

update: $(obj-update)

# Default configurations
.config:
	cp config/defconfig $@
	$(MAKE) oldconfig

$(LINUXDIR)/.config:
	cp $(LINUXDIR)/arch/mips/defconfig-bcm947xx $@

CP:=cp -fpR
INSTALL_BIN:=install -m0755
INSTALL_DIR:=install -d -m0755
INSTALL_DATA:=install -m0644
INSTALL_CONF:=install -m0600

include rules/_cmake.mk
include rules/matrixssl.mk
include rules/ntpd.mk
include rules/chrony.mk
include rules/htpdate.mk
include rules/gpsd.mk
include rules/libpng.mk
include rules/libgd.mk
include rules/libmcrypt.mk
include rules/php8.mk
include rules/openssl.mk
include rules/rp-l2tp.mk
include rules/libpcap.mk
include rules/eou.mk
include rules/ses.mk
include rules/libnet.mk
include rules/httpd.mk
include rules/jansson.mk
include rules/rc.mk
include rules/www.mk
include rules/bridge.mk
include rules/busybox.mk
include rules/dnsmasq.mk
include rules/nettle.mk
include rules/sodium.mk
include rules/dnscrypt.mk
include rules/iptables-new.mk
include rules/iptables.mk
include rules/netconf.mk
include rules/pptpd.mk
include rules/sstp.mk
include rules/pipsec.mk
include rules/snmp.mk
include rules/wifidog.mk
include rules/wol.mk
include rules/dhcp-forwarder.mk
include rules/dhcpv6.mk
include rules/rtp-proxy.mk
include rules/shat.mk
include rules/radauth.mk
include rules/samba.mk
include rules/samba3.mk
include rules/ntfs-3g.mk
include rules/mmc.mk
include rules/mmc-fonera.mk
include rules/mmc-ixp4xx.mk
include rules/openser.mk				
include rules/nocat.mk
include rules/http-redirect.mk
include rules/smtp-redirect.mk		
include rules/chillispot.mk
include rules/frottle.mk
include rules/ebtables.mk
include rules/ntpclient.mk
include rules/pppoe.mk
include rules/udhcpd.mk
include rules/xl2tpd.mk
include rules/wlconf.mk
include rules/vlan.mk
include rules/brcm_nas.mk
include rules/tftpd.mk
include rules/cron.mk
include rules/pptp-client.mk
include rules/libtalloc.mk
include rules/freeradius3.mk
include rules/netstatnat.mk
include rules/pppd.mk
include rules/ppp.mk
include rules/ipupdate.mk
include rules/inadyn.mk
include rules/zebra.mk
include rules/bird.mk
include rules/shorewall.mk
include rules/bpalogin.mk
include rules/dropbear.mk
include rules/iproute2.mk
include rules/igmp-proxy.mk
include rules/udpxy.mk
include rules/parprouted.mk

include rules/hostapd.mk
ifeq ($(CONFIG_MADWIFI),y)
include rules/hostapd-wps.mk
#include rules/hostapd2.mk
#include rules/wpa_supplicant2.mk
else
include rules/hostapd2.mk
include rules/wpa_supplicant2.mk
endif
include rules/madwifi.mk
#include rules/sputnik.mk
include rules/openvpn.mk
include rules/net-tools.mk
include rules/wireless-tools.mk
include rules/olsrd.mk
include rules/batman-adv.mk
include rules/tcpdump.mk
include rules/nvram.mk
include rules/fdisk.mk
include rules/moxa.mk
include rules/btrfsprogs.mk
include rules/xfsprogs.mk
include rules/e2fsprogs.mk
include rules/ncurses.mk
include rules/iftop.mk
include rules/iptraf.mk
include rules/squid.mk
include rules/proftpd.mk
include rules/zabbix.mk
include rules/ippd.mk
include rules/pcmcia.mk
include rules/gcom.mk
include rules/comgt.mk
include rules/setserial.mk
include rules/microcom.mk
include rules/nvtlstatus.mk
include rules/ctorrent.mk
#include rules/milkfish.mk
include rules/slang.mk
include rules/mc.mk
include rules/readline.mk
include rules/quagga.mk
include rules/frr.mk
include rules/bmon.mk
include rules/sercd.mk
include rules/ser2net.mk
include rules/p910nd.mk
include rules/ethtool.mk
include rules/openlldp.mk
include rules/ipsec-tools.mk
include rules/libunwind.mk
include rules/strace.mk
include rules/util-linux.mk
include rules/asterisk.mk
include rules/zaptel.mk
ifeq ($(CONFIG_IPERF),y)
include rules/iperf.mk
endif
include rules/wavesat.mk
include rules/libshared.mk
include rules/upnp.mk
include rules/services.mk
include rules/utils.mk
include rules/libutils.mk
include rules/zlib.mk
include rules/oled.mk
include rules/pound.mk
include rules/radvd.mk
include rules/nprobe.mk
include rules/mtr.mk
include rules/snoop.mk
include rules/aoss.mk
include rules/ap-serv.mk
include rules/tolapai.mk
include rules/buffalo-flash.mk
include rules/relayd.mk
include mac80211-rules/ath9k.mk
include rules/libnltiny.mk
include rules/hotplug2.mk
include rules/dsl_cpe_control.mk
include rules/atm.mk
include rules/aiccu.mk
include rules/l2tpv3tun.mk
include rules/rt3062.mk
include rules/libudev.mk
include rules/usbip.mk
include rules/glib.mk
include rules/libxml2.mk
#include rules/xtables-addons.mk
#snort
include rules/libnfnetlink.mk
include rules/libnetfilter_queue.mk
include rules/libnetfilter_log.mk
include rules/libdnet.mk
include rules/daq.mk
include rules/pcre.mk
include rules/pcre2.mk
include rules/snort.mk
include rules/swconfig.mk
include rules/dlna.mk
include rules/polarssl.mk
include rules/libubox.mk
include rules/nld.mk
include rules/nsmd.mk
include rules/picocom.mk
include rules/json-c.mk
include rules/ubus.mk
include rules/usteer.mk
include rules/dawn.mk
include rules/uqmi.mk
include rules/ipeth.mk
include rules/dns_responder.mk
include rules/minidlna.mk
include rules/nrpe.mk
include rules/links.mk
include rules/lighttpd.mk
include rules/nextmediaextra.mk
include rules/libqmi.mk
include rules/strongswan.mk
include rules/gmp.mk
include rules/sqlite.mk
include rules/privoxy.mk
include rules/kobs-ng.mk
include rules/lighttpd.mk
include rules/curl.mk
include rules/transmission.mk
include rules/libevent.mk
include rules/tor.mk
include rules/emf.mk
include rules/softflowd.mk
include rules/python.mk
include rules/nmap.mk
include rules/arpalert.mk
include rules/unbound.mk
include rules/ubi-utils.mk
include rules/libffi.mk
include rules/lzo.mk
include rules/java.mk
include rules/softether.mk
include rules/f2fs-tools.mk
-include rules/anchorfree.mk
include rules/speedchecker.mk
include rules/shownf.mk
include rules/lsof.mk
include rules/wolfssl.mk
include rules/powertop.mk
include rules/libmbim.mk
include rules/libmnl.mk
include rules/wireguard.mk
include rules/screen.mk
include rules/aircrack-ng.mk
include rules/ddrescue.mk
include rules/xz.mk
include rules/zstd.mk
include rules/exfat-utils.mk
include rules/dosfstools.mk
include rules/libtirpc.mk
include rules/zfs.mk
include rules/libzip.mk
include rules/qrencode.mk
include rules/iozone.mk
include rules/lvm2.mk
include rules/keyutils.mk
include rules/nfs-utils.mk
include rules/rpcbind.mk
include rules/krb5.mk
include rules/rsync.mk
include rules/libyang.mk
include rules/smartmontools.mk
include rules/sispmctl.mk
include rules/samba4.mk
include rules/gnutls.mk
include rules/libnl.mk
include rules/smbd.mk
include rules/irqbalance.mk
include rules/smartdns.mk
include rules/nginx.mk
include rules/mrp.mk
include rules/cfm.mk
include rules/htop.mk
include rules/ipset.mk
include rules/sdparm.mk
#ifeq ($(CONFIG_MDNS_UTILS),y)
include rules/dbus.mk
#endif
include rules/expat.mk
include rules/libdaemon.mk
include rules/avahi.mk
include rules/prepare.mk
include rules/libcares.mk
include rules/antaira-agent.mk
include rules/boinc.mk
include rules/musl.mk
include rules/microhttpd.mk
include rules/nodogsplash.mk
include rules/opennds.mk
include rules/btop.mk
include rules/envtools.mk

#
# Generic rules
#

%:
	[ ! -d $* ] || make -C $*

%-distclean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-configure:
	-[ ! -d $* ] || $(MAKE) -C $* configure

%-clean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-install:
	[ ! -d $* ] || make -C $* install INSTALLDIR=$(INSTALLDIR)/$*


$(obj-y) $(obj-m) $(obj-n) $(obj-clean) $(obj-install): dummy

