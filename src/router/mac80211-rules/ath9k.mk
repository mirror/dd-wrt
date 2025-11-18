include $(TOP)/.config
MAC80211_PATH=compat-wireless
ifeq ($(CONFIG_IPQ6018),y)
MAC80211_PATH=compat-wireless-nss
endif
ifeq ($(CONFIG_X86),y)
MAC80211_PATH=compat-wireless-nss
endif
ifeq ($(CONFIG_ALPINE),y)
MAC80211_PATH=compat-wireless-nss
endif
ifeq ($(CONFIG_NEWPORT),y)
MAC80211_PATH=compat-wireless-nss
endif
#ifeq ($(CONFIG_IPQ806X),y)
#MAC80211_PATH=compat-wireless-nss
#endif
ifeq ($(wildcard $(TOP)/$(MAC80211_PATH)/config.mk),) 
	include $(TOP)/mac80211-rules/ath9k-kconfig.mk	
else
	include $(TOP)/mac80211-rules/ath9k-nokconfig.mk	
endif
