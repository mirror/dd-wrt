# Makefile for the clients using the NSS driver

ccflags-y := -I$(obj) -I$(obj)/..

export BUILD_ID = \"Build Id: $(shell date +'%m/%d/%y, %H:%M:%S')\"
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

qca-nss-tun6rd-objs := nss_connmgr_tun6rd.o

ccflags-y += -DNSS_TUN6RD_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror

KERNELVERSION := $(word 1, $(subst ., ,$(KERNELVERSION))).$(word 2, $(subst ., ,$(KERNELVERSION)))

obj-$(bridge-mgr)+= bridge/
obj-$(capwapmgr)+= capwapmgr/
obj-$(dtlsmgr)+= dtls/$(DTLSMGR_DIR)/
obj-$(gre)+= gre/
#obj-$(ipsecmgr)+= ipsecmgr/$(IPSECMGR_DIR)/
#obj-$(ipsecmgr-klips)+= ipsecmgr/$(IPSECMGR_DIR)/plugins/klips/
#obj-$(ipsecmgr-xfrm)+= ipsecmgr/$(IPSECMGR_DIR)/plugins/xfrm/
obj-$(l2tpv2)+= l2tp/l2tpv2/
obj-$(lag-mgr)+= lag/
obj-$(map-t)+= map/map-t/
obj-$(portifmgr)+= portifmgr/
obj-$(pptp)+= pptp/
obj-$(profile)+= profiler/
obj-$(tunipip6)+= tunipip6/
obj-$(tun6rd)+= qca-nss-tun6rd.o
obj-$(qdisc)+= nss_qdisc/
obj-$(vlan-mgr)+= vlan/
obj-$(vxlanmgr)+= vxlanmgr/
obj-$(pvxlanmgr)+= pvxlanmgr/
obj-$(pppoe)+= pppoe/
obj-$(ovpn-mgr)+= openvpn/
obj-$(eogremgr)+= eogremgr/
obj-$(clmapmgr)+= clmapmgr/
obj-$(match)+= match/
obj-$(tlsmgr)+= tls/
obj-$(mirror)+= mirror/
obj-$(mscs)+= mscs/
obj-$(wifi-meshmgr)+= wifi_meshmgr/

#NSS NETLINK
obj-$(netlink)+= netlink/

obj ?= .
