# Makefile for the PPE TUNIPIP6 client

ccflags-y += -I$(obj)/../../exports -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_TUNIPIP6_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe-tunipip6.o
qca-nss-ppe-tunipip6-objs := nss_ppe_tunipip6.o

ccflags-y += -DNSS_PPE_TUNIPIP6_DEBUG_LEVEL=3
ccflags-y += -Wall -Werror
