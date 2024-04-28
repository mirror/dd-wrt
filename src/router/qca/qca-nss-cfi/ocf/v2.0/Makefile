# OCF specifc NSS cfi Makefile

NSS_CFI_MOD_NAME=qca-nss-cfi-ocf

obj-m += $(NSS_CFI_MOD_NAME).o
$(NSS_CFI_MOD_NAME)-objs = nss_cfi_ocf.o

obj ?= .

ccflags-y += -I$(obj)/../../ocf
ccflags-y += -I$(obj)/../../nss-crypto/include
ccflags-y += -I$(obj)/../../include
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_CFI_DEBUG_LEVEL=3
ccflags-y += -Werror

ifneq (, $(findstring ipq807x, $(SoC)))
ccflags-y += -DNSS_CFI_IPQ807X_SUPPORT
endif

ifneq (, $(findstring ipq60xx, $(SoC)))
ccflags-y += -DNSS_CFI_IPQ60XX_SUPPORT
endif
