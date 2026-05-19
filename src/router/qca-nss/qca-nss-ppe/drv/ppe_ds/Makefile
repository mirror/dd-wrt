# Makefile for PPE DS driver

ccflags-y := -I$(obj)/exports -I$(obj)/..
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"
ccflags-y += -Wall -Werror
ccflags-y += -DPPE_DS_DEBUG_LEVEL=0

ifeq ($(SoC),$(filter $(SoC),ipq53xx))
ccflags-y += -DNSS_PPE_IPQ53XX
endif

obj-m += qca-nss-ppe-ds-$(SoC).o
qca-nss-ppe-ds-$(SoC)-objs :=  ppe_ds_main.o  ppe_ds.o ppe_ds_stats.o
