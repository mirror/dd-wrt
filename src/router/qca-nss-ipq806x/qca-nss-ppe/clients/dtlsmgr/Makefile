# Makefile for DTLS manager

ccflags-y := -I$(obj)/../../exports -I$(obj)/.. -I$(obj)/../../drv/exports
ccflags-y += -DNSS_DTLSMGR_DEBUG_LEVEL=0
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror

obj-m += qca-nss-dtlsmgr.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx_dev.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx_stats.o
