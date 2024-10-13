# Makefile for TLS manager

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../exports
#ccflags-y += -DNSS_TLSMGR_DEBUG_DUMP
ccflags-y += -DNSS_TLSMGR_DEBUG_LEVEL=3
ccflags-y += -DNSS_TLSMGR_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y) SoC=$(SoC)'\"
ccflags-y += -Wall -Werror

obj-m += qca-nss-tlsmgr-$(SoC).o
qca-nss-tlsmgr-$(SoC)-objs += nss_tlsmgr.o
qca-nss-tlsmgr-$(SoC)-objs += nss_tlsmgr_buf.o
qca-nss-tlsmgr-$(SoC)-objs += nss_tlsmgr_crypto.o
qca-nss-tlsmgr-$(SoC)-objs += nss_tlsmgr_ctx.o
qca-nss-tlsmgr-$(SoC)-objs += nss_tlsmgr_tun.o
