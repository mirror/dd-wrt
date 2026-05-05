# ###################################################
# # Makefile for the NSS EIP197 driver
# ###################################################

NSS_EIP_MOD_NAME=qca-nss-eip

NSS_EIP_DRIVER_INCLUDE = -I$(obj)/../exports
ccflags-y += $(NSS_EIP_DRIVER_INCLUDE)

ccflags-y += -Wall -Werror
ccflags-y += -DNSS_EIP_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S') SoC=$(SoC)'\"

obj-m += $(NSS_EIP_MOD_NAME)-$(SoC).o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_ctx.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_dma.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_dma_hy.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_hw.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_skcipher.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_ahash.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_aead.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_ipsec.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_ipsec_dec.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tr_ipsec_enc.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_tk.o
$(NSS_EIP_MOD_NAME)-$(SoC)-objs += eip_flow.o
obj ?= .
