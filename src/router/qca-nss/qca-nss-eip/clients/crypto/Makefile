###################################################
# Makefile for the NSS EIP CRYPTO client
###################################################


EIP_MOD_NAME = qca-nss-eip-crypto

obj-m += $(EIP_MOD_NAME)-$(SoC).o
$(EIP_MOD_NAME)-$(SoC)-objs += eip_crypto.o
$(EIP_MOD_NAME)-$(SoC)-objs += eip_crypto_aead.o
$(EIP_MOD_NAME)-$(SoC)-objs += eip_crypto_skcipher.o
$(EIP_MOD_NAME)-$(SoC)-objs += eip_crypto_ahash.o

EIP_CRYPTO_INCLUDE = -I$(obj)/../../driver/exports

obj ?= .

ccflags-y += $(EIP_CRYPTO_INCLUDE)
ccflags-y += -Wall -Werror

