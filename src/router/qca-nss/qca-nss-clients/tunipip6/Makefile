# Makefile for tunipip6 client
ccflags-y += -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_TUNIPIP6_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
obj-m += qca-nss-tunipip6.o
qca-nss-tunipip6-objs := nss_connmgr_tunipip6.o nss_connmgr_tunipip6_sysctl.o nss_connmgr_tunipip6_stats.o
ifneq ($(findstring 4.4, $(KERNELVERSION)),)
ccflags-y += -DDRAFT03_SUPPORT
endif
