obj-$(CONFIG_RAETH) += raeth.o
raeth-objs := raether.o ra_mac.o mcast.o mii_mgr.o 

ifeq ($(CONFIG_RTL8366_SWITCH),y)
raeth-objs  += rtl_smi.o rtl8366s_api.o  rtl8366s_asicdrv.o
EXTRA_CFLAGS   += -D_LITTLE_ENDIAN
endif

ifeq ($(CONFIG_RAETH_QOS),y)
raeth-objs += ra_qos.o
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
raeth-objs += rt3052_access.o
endif

ifeq ($(CONFIG_ETHTOOL),y)
ifeq ($(CONFIG_RAETH_ROUTER),y)
raeth-objs += ra_ethtool.o
else
ifeq ($(CONFIG_RT_3052_ESW),y)
raeth-objs += ra_ethtool.o
endif
endif
endif

ifeq ($(CONFIG_RAETH_NETLINK),y)
raeth-objs += ra_netlink.o
endif

ifeq ($(CONFIG_RAETH_NAPI),y)
EXTRA_CFLAGS   += -DCONFIG_RAETH_NAPI
endif

ifeq ($(CONFIG_RT2880_BRIDGING_ONLY),y)
EXTRA_CFLAGS   += -DCONFIG_RT2880_BRIDGING_ONLY
endif

ifeq ($(CONFIG_RT2880_ENHANCE),y)
EXTRA_CFLAGS += -DCONFIG_RT2880_ENHANCE
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_RAETH_JUMBOFRAME),y)
EXTRA_CFLAGS += -DCONFIG_RAETH_JUMBOFRAME
endif

ifeq ($(CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

