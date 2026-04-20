# Cryptoapi specifc NSS cfi Makefile

NSS_CRYPTOAPI_MOD_NAME=qca-nss-cfi-cryptoapi-$(SoC)

obj-m += $(NSS_CRYPTOAPI_MOD_NAME).o
$(NSS_CRYPTOAPI_MOD_NAME)-objs = nss_cryptoapi.o
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_aead.o
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_debugfs.o
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_ablk.o
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_ahash.o

obj ?= .

ccflags-y += -I$(obj)/../../include
ccflags-y += -I$(obj)/../exports
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_CFI_DEBUG_LEVEL=3
ccflags-y += -Werror
#ccflags-y += -DCONFIG_NSS_CFI_DBG
