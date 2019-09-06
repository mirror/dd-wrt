include $(TOP)/.config

MAC80211_PATH=compat-wireless

ifeq ($(wildcard $(TOP)/$(MAC80211_PATH)/config.mk),) 
	include $(TOP)/private/ath9k-rules/ath9k-kconfig.mk	
else
	include $(TOP)/private/ath9k-rules/ath9k-nokconfig.mk	
endif
