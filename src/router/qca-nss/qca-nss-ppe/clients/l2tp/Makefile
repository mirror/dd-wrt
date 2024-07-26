# Makefile for the PPE L2TP client

ccflags-y += -I$(obj)/../../exports -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_L2TP_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe-l2tp.o
qca-nss-ppe-l2tp-objs := nss_ppe_l2tp.o

ccflags-y += -DNSS_PPE_L2TP_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror