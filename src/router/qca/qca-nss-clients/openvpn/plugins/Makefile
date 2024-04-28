
# Makefile for the ovpn plugins

ccflags-y := $(NSS_CCFLAGS) -I$(obj) -I$(obj)/../../exports -I$(obj)/include -I$(obj)/../src
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_OVPNMGR_DEBUG_LEVEL=0
ccflags-y += -DNSS_OVPN_LINK_DEBUG_LEVEL=0
ccflags-y += -Werror

obj-m += qca-nss-ovpn-link.o
qca-nss-ovpn-link-objs := nss_ovpn_link.o
obj ?= .
