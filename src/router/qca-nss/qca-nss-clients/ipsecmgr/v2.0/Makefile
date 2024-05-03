# Makefile for the ipsecmgr v2.0 client

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../../exports
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror

obj-m += qca-nss-ipsecmgr.o
qca-nss-ipsecmgr-objs := nss_ipsecmgr.o
qca-nss-ipsecmgr-objs += nss_ipsecmgr_ref.o
qca-nss-ipsecmgr-objs += nss_ipsecmgr_tunnel.o
qca-nss-ipsecmgr-objs += nss_ipsecmgr_ctx.o
qca-nss-ipsecmgr-objs += nss_ipsecmgr_sa.o
qca-nss-ipsecmgr-objs += nss_ipsecmgr_flow.o
ccflags-y += -DNSS_IPSECMGR_DEBUG_LEVEL=3

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DNSS_IPSECMGR_PPE_SUPPORT
ccflags-y += -DNSS_IPSECMGR_DMA_SUPPORT
endif
