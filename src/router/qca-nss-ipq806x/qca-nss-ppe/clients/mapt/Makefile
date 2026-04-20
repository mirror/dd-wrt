# Makefile for the PPE MAPT client

ccflags-y += -I$(obj)/../../exports -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_MAPT_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe-mapt.o
qca-nss-ppe-mapt-objs := nss_ppe_mapt.o

ccflags-y += -DNSS_PPE_MAPT_DEBUG_LEVEL=3
ccflags-y += -Wall -Werror
