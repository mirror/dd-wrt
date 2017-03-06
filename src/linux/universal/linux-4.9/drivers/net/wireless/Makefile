#
# Makefile for the Linux Wireless network device drivers.
#

obj-$(CONFIG_WLAN_VENDOR_ADMTEK) += admtek/
obj-$(CONFIG_WLAN_VENDOR_ATH) += ath/
obj-$(CONFIG_WLAN_VENDOR_ATMEL) += atmel/
obj-$(CONFIG_WLAN_VENDOR_BROADCOM) += broadcom/
obj-$(CONFIG_WLAN_VENDOR_CISCO) += cisco/
obj-$(CONFIG_WLAN_VENDOR_INTEL) += intel/
obj-$(CONFIG_WLAN_VENDOR_INTERSIL) += intersil/
obj-$(CONFIG_WLAN_VENDOR_MARVELL) += marvell/
obj-$(CONFIG_WLAN_VENDOR_MEDIATEK) += mediatek/
obj-$(CONFIG_WLAN_VENDOR_RALINK) += ralink/
obj-$(CONFIG_WLAN_VENDOR_REALTEK) += realtek/
ifeq ($(CONFIG_RALINK_DEVICE),y)
obj-$(CONFIG_RT2860V2_AP)       += rt3352/rt2860v2_ap/
obj-$(CONFIG_RT2860V2_STA)      += rt3352/rt2860v2_sta/
endif
ifeq ($(CONFIG_SOC_MT7620_OPENWRT),y)
obj-$(CONFIG_RT2860V2_AP)       += rt7620/rt2860v2_ap/
obj-$(CONFIG_RT2860V2_STA)      += rt7620/rt2860v2_sta/
obj-$(CONFIG_RTPCI_AP)      += rt5592/
obj-$(CONFIG_RLT_AP_SUPPORT)      += rt7612/rlt_wifi_ap/
endif
obj-$(CONFIG_WLAN_VENDOR_RSI) += rsi/
obj-$(CONFIG_WLAN_VENDOR_ST) += st/
obj-$(CONFIG_WLAN_VENDOR_TI) += ti/
obj-$(CONFIG_WLAN_VENDOR_ZYDAS) += zydas/

# 16-bit wireless PCMCIA client drivers
obj-$(CONFIG_PCMCIA_RAYCS)	+= ray_cs.o
obj-$(CONFIG_PCMCIA_WL3501)	+= wl3501_cs.o

obj-$(CONFIG_USB_NET_RNDIS_WLAN)	+= rndis_wlan.o

obj-$(CONFIG_MAC80211_HWSIM)	+= mac80211_hwsim.o
