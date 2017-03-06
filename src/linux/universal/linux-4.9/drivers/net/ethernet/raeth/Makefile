obj-$(CONFIG_RAETH) += raeth.o
raeth-objs := raether.o ra_mac.o mii_mgr.o ra_rfrw.o

#EXTRA_CFLAGS   += -DCONFIG_RAETH_MULTIPLE_RX_RING

ifeq ($(CONFIG_RTL8366_SWITCH),y)
raeth-objs  += rtl_smi.o rtl8366s_api.o  rtl8366s_asicdrv.o
EXTRA_CFLAGS   += -D_LITTLE_ENDIAN
endif

ifeq ($(CONFIG_RTL8366RB_SWITCH),y)
raeth-objs  += rtl_smi.o rb/rtl8366rb_api.o  rb/rtl8368s_asicdrv.o rb/rmain.o
EXTRA_CFLAGS   += -D_LITTLE_ENDIAN
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
raeth-objs += rt3052_access.o
endif

ifeq ($(CONFIG_RAETH_QOS),y)
raeth-objs += ra_qos.o
endif

ifeq ($(CONFIG_ETHTOOL),y)
ifeq ($(CONFIG_RAETH_ROUTER),y)
raeth-objs += ra_ethtool.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3052_MP2),y)
raeth-objs += mcast.o
endif

ifeq ($(CONFIG_RAETH_NETLINK),y)
raeth-objs += ra_netlink.o
endif

ifeq ($(CONFIG_RAETH_GMAC2),y)
EXTRA_CFLAGS += -DCONFIG_PSEUDO_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_TASKLET_WORKQUEUE_SW),y)
EXTRA_CFLAGS   += -DTASKLET_WORKQUEUE_SW
endif


EXTRA_CFLAGS	+= -Idrivers/net/ethernet/raeth/ -D_LITTLE_ENDIAN
EXTRA_CFLAGS	+= -I. -D_LITTLE_ENDIAN
EXTRA_CFLAGS	+= -Idrivers/net/ethernet/raeth/ar8316/ 

obj-$(CONFIG_RTL8367M)		+= rtk_api.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_acl.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_cputag.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_dot1x.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_eee.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_eeelldp.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_fc.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_green.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_igmp.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_inbwctrl.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_interrupt.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_led.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_lut.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_meter.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_mib.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_mirror.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_misc.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_oam.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_phy.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_port.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_portIsolation.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_qos.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_rldp.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_rma.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_rrcp.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_scheduling.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_specialCongest.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_storm.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_svlan.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_trunking.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_unknownMulticast.o
obj-$(CONFIG_RTL8367M)		+= rtl8370_asicdrv_vlan.o


obj-$(CONFIG_RTL8367M)		+= rtl8367m_drv.o
obj-$(CONFIG_AR8316)		+= ar8316_drv.o

