# Makefile for pppoe client
ccflags-y += -I$(obj)/../exports -I$(obj)/.. -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_PPPOE_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
obj-m += qca-nss-ppe-pppoe-mgr.o
qca-nss-ppe-pppoe-mgr-objs := nss_ppe_pppoe_mgr.o

#
# TODO: Remove the following check when BONDING support is added for PPE
#
ifeq ($(findstring 6.1, $(KERNELVERSION)),)
ifneq (,$(filter $(CONFIG_BONDING),y m))
ccflags-y += -DBONDING_SUPPORT
endif
endif
