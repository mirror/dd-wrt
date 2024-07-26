# Makefile for PPE driver

ccflags-y := -I$(obj)/../exports -I$(obj)/../ppe_drv/tun/ -I$(obj)/..
ccflags-y += -DPPE_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror
ccflags-y += -DPPE_TUN_DEBUG_LEVEL=3

obj-m += qca-nss-ppe-tun.o
qca-nss-ppe-tun-objs := ppe_tun.o
