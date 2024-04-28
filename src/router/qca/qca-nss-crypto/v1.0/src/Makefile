# ###################################################
# # Makefile for the NSS driver
# ###################################################

NSS_CRYPTO_MOD_NAME=qca-nss-crypto

#ccflags-y += -DCONFIG_NSS_CRYPTO_DBG
ccflags-y += -DCONFIG_NSS_CRYPTO_FORCE_UNCACHE=1
ccflags-y += -DNSS_CRYPTO_DEBUG_LEVEL=2
ccflags-y += -Werror
ccflags-y += -DNSS_CRYPTO_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S')'\"

obj-m += $(NSS_CRYPTO_MOD_NAME).o
$(NSS_CRYPTO_MOD_NAME)-objs = nss_crypto_if.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_ctrl.o
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_debugfs.o

ifneq ($(findstring 3.4, $(KERNELVERSION)),)
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_platform.o
else
$(NSS_CRYPTO_MOD_NAME)-objs += nss_crypto_dtsi.o
endif

obj ?= .
ccflags-y += -I$(obj)/include -I$(obj)/
