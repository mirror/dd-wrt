#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#

ifdef CONFIG_BCM_KF_WL
ifneq ($(strip $(CONFIG_BCM_WLAN)),)
EXTRA_CFLAGS    += -DPKTC
EXTRA_CFLAGS    += -DDSLCPE -DBCMDRIVER
BRCM_CHIP=4908
# File: bcmdrivers/opensource/char/plat-bcm
#
# Makefile for the BCM63xx/68xx  platform drivers
EXTRA_CFLAGS	+= -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared -I$(srctree)/arch/arm64/brcm/include/pmc
endif
endif # BCM_KF

obj-$(CONFIG_BRIDGE) += bridge.o

bridge-y	:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o br_netlink.o

bridge-$(CONFIG_SYSFS) += br_sysfs_if.o br_sysfs_br.o

bridge-$(subst m,y,$(CONFIG_BRIDGE_NETFILTER)) += br_nf_core.o

br_netfilter-y := br_netfilter_hooks.o
br_netfilter-$(subst m,y,$(CONFIG_IPV6)) += br_netfilter_ipv6.o
obj-$(CONFIG_BRIDGE_NETFILTER) += br_netfilter.o

bridge-$(CONFIG_BRIDGE_IGMP_SNOOPING) += br_multicast.o br_mdb.o

bridge-$(CONFIG_BRIDGE_VLAN_FILTERING) += br_vlan.o

obj-$(CONFIG_NETFILTER) += netfilter/

ifdef CONFIG_BCM_KF_MCAST
ifneq ($(strip $(CONFIG_BCM_MCAST))$(strip $(CONFIG_BCM_MCAST_MODULE)),)
bridge-y += br_bcm_mcast.o
endif
endif # BCM_KF

ifdef CONFIG_BCM_KF_NETFILTER
bridge-y += br_notifier.o
endif # BCM_KF

ifdef CONFIG_BCM_KF_RUNNER
ifdef CONFIG_BCM_RDPA_BRIDGE
bridge-y += br_fp.o
endif # CONFIG_BCM_RDPA_BRIDGE
endif # BCM_KF
