###################################################
# Makefile for the NSS data plane driver
###################################################

obj ?= .

obj-m += qca-nss-dp-$(SoC).o

qca-nss-dp-$(SoC)-objs += nss_dp_attach.o \
		   nss_dp_ethtools.o \
		   hal/soc_ops/$(SoC)/nss_$(SoC).o

ifneq ($(CONFIG_NET_SWITCHDEV),)
qca-nss-dp-$(SoC)-objs += nss_dp_switchdev.o
endif

ifeq ($(SoC),$(filter $(SoC),ipq53xx))
qca-nss-dp-$(SoC)-objs += nss_dp_eawtp.o
endif

NSS_DP_INCLUDE = -I$(obj)/include -I$(obj)/exports -I$(obj)/hal/include \
		 -I$(obj)/hal/dp_ops/include

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq60xx))
qca-nss-dp-$(SoC)-objs += hal/dp_ops/edma_dp/edma_v1/edma_cfg.o \
		   hal/dp_ops/edma_dp/edma_v1/edma_data_plane.o \
		   hal/dp_ops/edma_dp/edma_v1/edma_tx_rx.o \
		   hal/gmac_ops/qcom/qcom_if.o \
		   hal/gmac_ops/syn/xgmac/syn_if.o
NSS_DP_INCLUDE += -I$(obj)/hal/dp_ops/edma_dp/edma_v1/include
ccflags-y += -DNSS_DP_PPE_SUPPORT -DNSS_DP_MAC_POLL_SUPPORT
endif

ifeq ($(SoC),$(filter $(SoC),ipq807x))
ccflags-y += -DNSS_DP_IPQ807X -DNSS_DP_EDMA_TX_SMALL_PKT_WAR
qca-nss-dp-$(SoC)-objs += nss_dp_main_alt.o
endif

ifeq ($(SoC),$(filter $(SoC),ipq60xx))
ccflags-y += -DNSS_DP_IPQ60XX
qca-nss-dp-$(SoC)-objs += nss_dp_main_alt.o
endif

ifeq ($(SoC),$(filter $(SoC),ipq53xx))
ccflags-y += -DNSS_DP_MHT_SW_PORT_MAP
ccflags-y += -DNSS_DP_CONFIG_RST
qca-nss-dp-$(SoC)-objs += nss_dp_main.o
endif

# Adding flag to enable physical port mirror support
# in PPE - To be enabled when mirror support is needed.
#ifeq ($(SoC),$(filter $(SoC),ipq95xx ipq53xx))
#ccflags-y += -DNSS_DP_PORT_MIRROR_EN
#endif

ifeq ($(SoC),$(filter $(SoC),ipq50xx))
qca-nss-dp-$(SoC)-objs += nss_dp_main.o
qca-nss-dp-$(SoC)-objs += hal/dp_ops/syn_gmac_dp/syn_dp_cfg_rx.o \
		   hal/dp_ops/syn_gmac_dp/syn_dp_cfg_tx.o \
		   hal/dp_ops/syn_gmac_dp/syn_dp_rx.o \
		   hal/dp_ops/syn_gmac_dp/syn_dp_tx.o \
		   hal/dp_ops/syn_gmac_dp/syn_dp.o \
		   hal/gmac_ops/syn/gmac/syn_if.o
NSS_DP_INCLUDE += -I$(obj)/hal/dp_ops/syn_gmac_dp/include
ccflags-y += -DNSS_DP_IPQ50XX -DNSS_DP_ENABLE_NAPI_GRO -DNSS_NO_PPE
endif

ifeq ($(SoC),$(filter $(SoC),ipq95xx ipq53xx))
qca-nss-dp-$(SoC)-objs += nss_dp_vp_main.o \
		   nss_dp_ethtool_priv.o \
		   hal/dp_ops/edma_dp/edma_v2/edma.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_cfg_rx.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_cfg_tx.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_debugfs.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_dp.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_dp_vp.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_misc.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_procfs.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_rx.o \
		   hal/dp_ops/edma_dp/edma_v2/edma_tx.o \
		   hal/gmac_ops/qcom/qcom_if.o \
		   hal/gmac_ops/syn/xgmac/syn_if.o

ifeq ($(dp-ppe-ds),y)
qca-nss-dp-$(SoC)-objs += hal/dp_ops/edma_dp/edma_v2/edma_ppeds.o
ccflags-y += -DNSS_DP_PPEDS_SUPPORT
endif
ifeq ($(dp-net-standby),y)
qca-nss-dp-$(SoC)-objs += nss_dp_netstandby.o
ccflags-y += -DNSS_DP_NETSTANDBY
endif
NSS_DP_INCLUDE += -I$(obj)/hal/dp_ops/edma_dp/edma_v2
NSS_DP_INCLUDE += -I$(obj)/hal/dp_ops/edma_dp/edma_v2/include
ccflags-y += -DNSS_DP_ENABLE_NAPI_GRO -DNSS_DP_VP_SUPPORT -DNSS_DP_EDMA_V2 -DNSS_DP_MAC_POLL_SUPPORT -DNSS_DP_SW_BR_OPS -DNSS_DP_ETHTOOL_MRR_OPS
endif

ifeq ($(SoC),$(filter $(SoC),ipq53xx))
ccflags-y += -DNSS_DP_IPQ53XX
endif

ifeq ($(SoC),$(filter $(SoC),ipq95xx))
ccflags-y += -DNSS_DP_IPQ95XX
endif

ccflags-y += $(NSS_DP_INCLUDE)
ccflags-y += -Wall -Werror
ccflags-y += -DEDMA_DEBUG_LEVEL=2
