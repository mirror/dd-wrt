# Makefile for DTLS manager

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../../exports
ccflags-y += -DNSS_DTLSMGR_DEBUG_LEVEL=0
ccflags-y += -DNSS_DTLSMGR_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S') SoC=$(SoC)'\"
ccflags-y += -Wall -Werror

obj-m += qca-nss-dtlsmgr.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx_dev.o
qca-nss-dtlsmgr-objs += nss_dtlsmgr_ctx_stats.o
