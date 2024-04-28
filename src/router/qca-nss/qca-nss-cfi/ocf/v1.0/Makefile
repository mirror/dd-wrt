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
