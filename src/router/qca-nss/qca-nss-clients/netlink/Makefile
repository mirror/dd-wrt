CAPWAP_ENABLED := $(strip $(if $(filter $(capwapmgr), y), 1 , 0))
IPSEC_ENABLED := $(strip $(if $(filter $(ipsecmgr), y), 1 , 0))
DTLS_ENABLED := $(strip $(if $(filter $(dtlsmgr), y), 1 , 0))

ccflags-y := -Wall -Werror
ccflags-y += -I$(obj)/include
ccflags-y += -I$(obj)/../exports
ccflags-y += -DNSS_NL_DEBUG_LEVEL=0
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

ccflags-y += -DCONFIG_NSS_NLIPV4=1
ccflags-y += -DCONFIG_NSS_NLIPV6=1
ccflags-y += -DCONFIG_NSS_NLOAM=1
ccflags-y += -DCONFIG_NSS_NLGRE_REDIR_FAMILY=1
ccflags-y += -DCONFIG_NSS_NLETHRX=1
ccflags-y += -DCONFIG_NSS_NLDYNAMIC_INTERFACE=1
ccflags-y += -DCONFIG_NSS_NLN2H=1
ccflags-y += -DCONFIG_NSS_NLIPV4_REASM=1
ccflags-y += -DCONFIG_NSS_NLIPV6_REASM=1
ccflags-y += -DCONFIG_NSS_NLWIFILI=1
ccflags-y += -DCONFIG_NSS_NLLSO_RX=1
ccflags-y += -DCONFIG_NSS_NLMAP_T=1
ccflags-y += -DCONFIG_NSS_NLPPPOE=1
ccflags-y += -DCONFIG_NSS_NLL2TPV2=1
ccflags-y += -DCONFIG_NSS_NLQRFS=1
ccflags-y += -DCONFIG_NSS_NLPPTP=1
ccflags-y += -DCONFIG_NSS_NLCAPWAP=${CAPWAP_ENABLED}
ccflags-y += -DCONFIG_NSS_NLIPSEC=${IPSEC_ENABLED}
ccflags-y += -DCONFIG_NSS_NLDTLS=${DTLS_ENABLED}

qca-nss-netlink-$(SoC)-objs := nss_nl.o
qca-nss-netlink-$(SoC)-objs += nss_nlgre_redir_family.o
qca-nss-netlink-$(SoC)-objs += nss_nlgre_redir_cmd.o
qca-nss-netlink-$(SoC)-objs += nss_nlgre_redir_cmn.o
qca-nss-netlink-$(SoC)-objs += nss_nlgre_redir.o
qca-nss-netlink-$(SoC)-objs += nss_nlgre_redir_lag.o
qca-nss-netlink-$(SoC)-objs += nss_nlipv4.o
qca-nss-netlink-$(SoC)-objs += nss_nlipv6.o
qca-nss-netlink-$(SoC)-objs += nss_nloam.o
qca-nss-netlink-$(SoC)-objs += nss_nlethrx.o
qca-nss-netlink-$(SoC)-objs += nss_nldynamic_interface.o
qca-nss-netlink-$(SoC)-objs += nss_nln2h.o
qca-nss-netlink-$(SoC)-objs += nss_nlipv4_reasm.o
qca-nss-netlink-$(SoC)-objs += nss_nlipv6_reasm.o
qca-nss-netlink-$(SoC)-objs += nss_nlwifili.o
qca-nss-netlink-$(SoC)-objs += nss_nllso_rx.o
qca-nss-netlink-$(SoC)-objs += nss_nlmap_t.o
qca-nss-netlink-$(SoC)-objs += nss_nlpppoe.o
qca-nss-netlink-$(SoC)-objs += nss_nll2tpv2.o
qca-nss-netlink-$(SoC)-objs += nss_nlpptp.o
qca-nss-netlink-$(SoC)-objs += nss_nlqrfs.o

ifneq (,$(filter $(capwapmgr), y))
qca-nss-netlink-$(SoC)-objs += nss_nlcapwap.o
endif

ifneq (,$(filter $(dtlsmgr), y))
qca-nss-netlink-$(SoC)-objs += nss_nldtls.o
endif

ifneq (,$(filter $(ipsecmgr), y))
qca-nss-netlink-$(SoC)-objs += nss_nlipsec.o
endif

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64))
ccflags-y += -DCONFIG_NSS_NLC2C_TX=1
ccflags-y += -DCONFIG_NSS_NLC2C_RX=1
qca-nss-netlink-$(SoC)-objs += nss_nlc2c_tx.o
qca-nss-netlink-$(SoC)-objs += nss_nlc2c_rx.o
else
ccflags-y += -DCONFIG_NSS_NLC2C_TX=0
ccflags-y += -DCONFIG_NSS_NLC2C_RX=0
endif

ifeq ($(SoC),$(filter $(SoC),ipq60xx ipq60xx_64 ipq807x ipq807x_64))
ccflags-y += -DCONFIG_NSS_NLEDMA=1
qca-nss-netlink-$(SoC)-objs += nss_nledma.o
else
ccflags-y += -DCONFIG_NSS_NLEDMA=0
endif

ifeq ($(SoC),$(filter $(SoC),ipq95xx ipq50xx))
ccflags-y += -DCONFIG_NSS_NLUDP_ST=1
qca-nss-netlink-$(SoC)-objs += nss_nludp_st.o
else
ccflags-y += -DCONFIG_NSS_NLUDP_ST=0
endif

obj-m += qca-nss-netlink-$(SoC).o
