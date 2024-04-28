# Klip plugin Makefile

NSS_IPSEC_MOD_NAME=qca-nss-ipsec-klips

obj-m += $(NSS_IPSEC_MOD_NAME).o
$(NSS_IPSEC_MOD_NAME)-objs = nss_ipsec_klips.o

obj ?= .
ccflags-y += -I$(obj)/../../include
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_IPSEC_KLIPS_DEBUG_LEVEL=3
ccflags-y += -DNSS_IPSEC_KLIPS_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Werror
ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64))
ccflags-y += -DNSS_CFI_IPQ807X_SUPPORT
endif

ifeq ($(SoC),$(filter $(SoC),ipq60xx ipq60xx_64))
ccflags-y += -DNSS_CFI_IPQ60XX_SUPPORT
endif
