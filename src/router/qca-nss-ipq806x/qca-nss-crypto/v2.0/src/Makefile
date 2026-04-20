# ###################################################
# # Makefile for the NSS driver
# ###################################################

NSS_CRYPTO_MOD_NAME=qca-nss-crypto-$(SoC)

#ccflags-y += -DNSS_CRYPTO_DBG
ccflags-y += -DNSS_CRYPTO_DEBUG_LEVEL=2
ccflags-y += -Wall -Werror
ccflags-y += -DNSS_CRYPTO_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y) SoC=$(SoC)'\"

ifneq (, $(findstring ipq807x, $(SoC)))
HAL_DIR=hal/ipq807x
crypto-hal-files += $(HAL_DIR)/nss_crypto_eip197.o $(HAL_DIR)/nss_crypto_hw.o $(HAL_DIR)/nss_crypto_eip197_init.o
ccflags-y += -DNSS_CRYPTO_IPQ807X_SUPPORT
endif

ifneq (, $(findstring ipq60xx, $(SoC)))
HAL_DIR=hal/ipq60xx
crypto-hal-files += $(HAL_DIR)/nss_crypto_eip197.o $(HAL_DIR)/nss_crypto_hw.o $(HAL_DIR)/nss_crypto_eip197_init.o
ccflags-y += -DNSS_CRYPTO_IPQ60XX_SUPPORT
endif

ifneq (, $(findstring ipq50xx, $(SoC)))
HAL_DIR=hal/ipq50xx
crypto-hal-files += $(HAL_DIR)/nss_crypto_ce5.o $(HAL_DIR)/nss_crypto_hw.o
ccflags-y += -DNSS_CRYPTO_IPQ50XX_SUPPORT
endif

ccflags-y += -I$(obj)/$(HAL_DIR)/

obj-m += $(NSS_CRYPTO_MOD_NAME).o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_ctrl.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_debugfs.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_transform.o
$(NSS_CRYPTO_MOD_NAME)-objs += $(crypto-hal-files)

obj ?= .
ccflags-y += -I$(obj)/include -I$(obj)/
