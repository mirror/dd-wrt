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

# Default configurations
.config:
	cp config/defconfig $@
	$(MAKE) oldconfig

$(LINUXDIR)/.config:
	cp $(LINUXDIR)/arch/mips/defconfig-bcm947xx $@


include rules/matrixssl.mk
include rules/ntpd.mk
include rules/gpsd.mk
include rules/php5.mk
include rules/openssl.mk
include rules/rp-l2tp.mk
include rules/libpcap.mk
include rules/eou.mk
include rules/ses.mk
include rules/libnet.mk
include rules/httpd.mk
include rules/rc.mk
include rules/www.mk
include rules/bridge.mk
include rules/busybox.mk
include rules/dnsmasq.mk
include rules/iptables.mk
include rules/netconf.mk
include rules/pptpd.mk
include rules/pipsec.mk
include rules/snmp.mk
include rules/wifidog.mk
include rules/wol.mk
include rules/dhcp-forwarder.mk
include rules/rtp-proxy.mk
include rules/shat.mk
include rules/radauth.mk
include rules/samba.mk
include rules/mmc.mk
include rules/mmc-fonera.mk
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
include rules/wlconf.mk
include rules/vlan.mk
include rules/brcm_nas.mk
include rules/tftpd.mk
include rules/cron.mk
include rules/pptp-client.mk
include rules/freeradius.mk
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
include rules/parprouted.mk
include rules/hostapd.mk
include rules/hostapd2.mk
include rules/wpa_supplicant.mk
include rules/wpa_supplicant2.mk
include rules/madwifi.mk
#include rules/madwifi_mimo.mk
include rules/sputnik.mk
include rules/openvpn.mk
include rules/net-tools.mk
include rules/wireless-tools.mk
include rules/olsrd.mk
include rules/tcpdump.mk
include rules/nvram.mk
include rules/fdisk.mk
include rules/moxa.mk
include rules/e2fsprogs.mk
include rules/ncurses.mk
include rules/iftop.mk
include rules/iptraf.mk
include rules/squid.mk
include rules/proftpd.mk
include rules/ippd.mk
include rules/pcmcia.mk
include rules/gcom.mk
include rules/setserial.mk
include rules/microcom.mk
include rules/nvtlstatus.mk
include rules/ctorrent.mk
include rules/milkfish.mk
include rules/quagga.mk
include rules/bmon.mk
include rules/ethtool.mk
include rules/openlldp.mk
include rules/ipsec-tools.mk
include rules/strace.mk
include rules/asterisk.mk
include rules/zaptel.mk
include rules/iperf.mk
include rules/wavesat.mk
include rules/libshared.mk
include rules/upnp.mk
include rules/services.mk
include rules/utils.mk
include rules/libutils.mk
include rules/zlib.mk
include rules/pound.mk

# Generic rules
#

%:
	[ ! -d $* ] || $(MAKE) -C $*  

%-distclean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-clean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-install:
	[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(INSTALLDIR)/$*


$(obj-y) $(obj-m) $(obj-n) $(obj-clean) $(obj-install): dummy

