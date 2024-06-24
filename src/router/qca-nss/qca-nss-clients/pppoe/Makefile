# Makefile for pppoe client
ccflags-y += -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_PPPOE_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
obj-m += qca-nss-pppoe.o
qca-nss-pppoe-objs := nss_connmgr_pppoe.o

#ifneq (,$(filter $(CONFIG_BONDING),y m))
#ccflags-y += -DBONDING_SUPPORT
#endif
