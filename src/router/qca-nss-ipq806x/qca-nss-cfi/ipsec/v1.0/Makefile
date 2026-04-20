# OCF specifc NSS cfi Makefile

NSS_IPSEC_MOD_NAME=qca-nss-ipsec

obj-m += $(NSS_IPSEC_MOD_NAME).o
$(NSS_IPSEC_MOD_NAME)-objs = nss_cfi_ipsec.o

obj ?= .
ccflags-y += -I$(obj)/../../include
ccflags-y += -I$(obj)/
ccflags-y += -DNSS_CFI_DEBUG_LEVEL=3
ccflags-y += -DNSS_CFI_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Werror