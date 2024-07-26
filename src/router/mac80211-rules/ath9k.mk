include $(TOP)/.config
ifeq ($(CONFIG_IPQ6018),y)
MAC80211_PATH=compat-wireless-nss
else
MAC80211_PATH=compat-wireless
endif
ifeq ($(wildcard $(TOP)/$(MAC80211_PATH)/config.mk),) 
	include $(TOP)/mac80211-rules/ath9k-kconfig.mk	
else
	include $(TOP)/mac80211-rules/ath9k-nokconfig.mk	
endif
