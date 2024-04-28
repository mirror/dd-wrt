# ###################################################
# # Makefile for the NSS driver
# ###################################################

NSS_CRYPTO_MOD_NAME=qca-nss-crypto

#ccflags-y += -DNSS_CRYPTO_DBG
ccflags-y += -DNSS_CRYPTO_DEBUG_LEVEL=2
ccflags-y += -Werror
ccflags-y += -DNSS_CRYPTO_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S') SoC=$(SoC)'\"

ifneq (, $(findstring ipq807x, $(SoC)))
SRC_DIR=hal/ipq807x
ccflags-y += -DNSS_CRYPTO_IPQ807X_SUPPORT
endif

ifneq (, $(findstring ipq60xx, $(SoC)))
SRC_DIR=hal/ipq60xx
ccflags-y += -DNSS_CRYPTO_IPQ60XX_SUPPORT
endif

ccflags-y += -I$(obj)/$(SRC_DIR)/

obj-m += $(NSS_CRYPTO_MOD_NAME).o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_ctrl.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_debugfs.o
$(NSS_CRYPTO_MOD_NAME)-objs += $(SRC_DIR)/nss_crypto_eip197.o
$(NSS_CRYPTO_MOD_NAME)-objs += $(SRC_DIR)/nss_crypto_hw.o
$(NSS_CRYPTO_MOD_NAME)-objs += $(SRC_DIR)/nss_crypto_eip197_init.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_transform.o

obj ?= .
ccflags-y += -I$(obj)/include -I$(obj)/
