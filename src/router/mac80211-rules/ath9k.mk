include $(TOP)/.config

MAC80211_PATH=compat-wireless

ifeq ($(wildcard $(TOP)/$(MAC80211_PATH)/config.mk),) 
	include $(TOP)/mac80211-rules/ath9k-kconfig.mk	
else
	include $(TOP)/mac80211-rules/ath9k-nokconfig.mk	
endif
