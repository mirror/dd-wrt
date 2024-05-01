# Makefile for the ovpnmgr client

ccflags-y := $(NSS_CCFLAGS) -I$(obj) -I$(obj)/../../exports -I$(obj)/include -I$(obj)/../plugins
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_OVPNMGR_DEBUG_LEVEL=0 -DNSS_OVPNMGR_DEBUG_ENABLE_PKT_DUMP=0
ccflags-y += -Wall -Werror

obj-m += qca-nss-ovpn-mgr.o
qca-nss-ovpn-mgr-objs := nss_ovpnmgr.o nss_ovpnmgr_app.o nss_ovpnmgr_tun.o nss_ovpnmgr_crypto.o \
			 nss_ovpnmgr_route.o nss_ovpnmgr_debugfs.o \
			 ../plugins/nss_ovpn_sk.o
obj ?= .
