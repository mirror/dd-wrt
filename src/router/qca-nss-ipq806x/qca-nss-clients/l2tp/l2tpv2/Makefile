# Makefile for l2tp client
ccflags-y += -I$(obj)/../../exports -I$(obj)/../..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_L2TP_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
ifneq (,$(filter $(CONFIG_L2TP),m y))
obj-m += qca-nss-l2tpv2-$(SoC).o
qca-nss-l2tpv2-$(SoC)-objs := nss_connmgr_l2tpv2.o nss_l2tpv2_stats.o
endif

