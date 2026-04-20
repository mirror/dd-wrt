# Makefile for the PPE GRE client

ccflags-y += -I$(obj)/../../exports -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_GRETAP_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe-gretap.o
qca-nss-ppe-gretap-objs := nss_ppe_gretap.o

ccflags-y += -DNSS_PPE_GRETAP_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror

