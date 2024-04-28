# Makefile for the clients using the NSS driver

ccflags-y := -I$(obj) -I$(obj)/..

export BUILD_ID = \"Build Id: $(shell date +'%m/%d/%y, %H:%M:%S')\"
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

qca-nss-tunipip6-objs := nss_connmgr_tunipip6.o
qca-nss-tun6rd-objs := nss_connmgr_tun6rd.o

ccflags-y += -DNSS_TUN6RD_DEBUG_LEVEL=0
ccflags-y += -DNSS_TUNIPIP6_DEBUG_LEVEL=0
ccflags-y += -Werror

obj-$(bridge-mgr)+= bridge/
obj-$(capwapmgr)+= capwapmgr/
obj-$(dtlsmgr)+= dtls/$(DTLSMGR_DIR)/
obj-$(gre)+= gre/
obj-$(ipsecmgr)+= ipsecmgr/$(IPSECMGR_DIR)/
obj-$(l2tpv2)+= l2tp/l2tpv2/
obj-$(lag-mgr)+= lag/
obj-$(map-t)+= map/map-t/
obj-$(portifmgr)+= portifmgr/
obj-$(pptp)+= pptp/
obj-$(profile)+= profiler/
obj-$(tunipip6)+= qca-nss-tunipip6.o
obj-$(tun6rd)+= qca-nss-tun6rd.o
obj-$(qdisc)+= nss_qdisc/
obj-$(vlan-mgr)+= vlan/
obj-$(pvxlanmgr)+= pvxlanmgr/
obj-$(pppoe)+= pppoe/
obj-$(ovpn-mgr)+= openvpn/

#NSS NETLINK
obj-$(netlink)+= netlink/

obj ?= .
