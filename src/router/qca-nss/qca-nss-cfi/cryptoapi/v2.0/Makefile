# Cryptoapi specifc NSS cfi Makefile

NSS_CRYPTOAPI_MOD_NAME=qca-nss-cfi-cryptoapi

obj-m += $(NSS_CRYPTOAPI_MOD_NAME).o
$(NSS_CRYPTOAPI_MOD_NAME)-objs = nss_cryptoapi.o
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_aead.o
ifneq "$(NSS_CRYPTOAPI_SKCIPHER)" "n"
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_skcipher.o
ccflags-y += -DNSS_CRYPTOAPI_SKCIPHER
endif
$(NSS_CRYPTOAPI_MOD_NAME)-objs += nss_cryptoapi_ahash.o

obj ?= .

ccflags-y += -I$(obj)/../../include
ccflags-y += -I$(obj)/../exports
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_CFI_DEBUG_LEVEL=3
ccflags-y += -Wall -Werror
#ccflags-y += -DCONFIG_NSS_CFI_DBG
ccflags-y += -DNSS_CFI_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y) SoC=$(SoC)'\"
ccflags-y += -DNSS_CRYPTOAPI_REQ_TIMEO_SECS=10
