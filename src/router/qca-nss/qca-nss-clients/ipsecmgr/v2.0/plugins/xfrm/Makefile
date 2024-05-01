# XFRM plugin Makefile

NSS_IPSEC_MOD_NAME=qca-nss-ipsec-xfrm

obj-m += $(NSS_IPSEC_MOD_NAME).o

$(NSS_IPSEC_MOD_NAME)-objs := nss_ipsec_xfrm.o
$(NSS_IPSEC_MOD_NAME)-objs += nss_ipsec_xfrm_tunnel.o
$(NSS_IPSEC_MOD_NAME)-objs += nss_ipsec_xfrm_sa.o
$(NSS_IPSEC_MOD_NAME)-objs += nss_ipsec_xfrm_flow.o

obj ?= .
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_IPSEC_XFRM_DEBUG_LEVEL=3
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror

ifeq ($(SoC),$(filter $(SoC),ipq50xx ipq50xx_64))
ccflags-y += -DNSS_IPSEC_XFRM_IPQ50XX
endif
