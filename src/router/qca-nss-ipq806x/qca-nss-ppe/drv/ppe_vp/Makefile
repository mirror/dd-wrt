# Makefile for PPE VP driver

ccflags-y := -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror
ccflags-y += -DPPE_VP_DEBUG_LEVEL=2

obj-m += qca-nss-ppe-vp.o
qca-nss-ppe-vp-objs := ppe_vp_base.o ppe_vp.o ppe_vp_rx.o ppe_vp_stats.o ppe_vp_tx.o
