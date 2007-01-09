O_TARGET := madwifi.o

subdir-$(CONFIG_ATHEROS)		+= ath
subdir-$(CONFIG_ATHEROS_RATE_SAMPLE)	+= ath_rate/sample
subdir-$(CONFIG_ATHEROS_RATE_ONOE)	+= ath_rate/onoe
subdir-$(CONFIG_ATHEROS_RATE_AMRR)	+= ath_rate/amrr
subdir-$(CONFIG_NET80211)		+= net80211

ifeq ($(CONFIG_ATHEROS),y)
ifeq ($(strip $(BUS)),AHB)
	obj-y += ath/ath_ahb.o
else
	obj-y += ath/ath_pci.o
endif
	obj-$(CONFIG_ATHEROS_RATE_SAMPLE) += ath_rate/sample/ath_rate.o
	obj-$(CONFIG_ATHEROS_RATE_ONOE) += ath_rate/onoe/ath_rate.o
	obj-$(CONFIG_ATHEROS_RATE_AMRR) += ath_rate/amrr/ath_rate.o
	obj-y += net80211/net80211.o
endif

include $(TOPDIR)/Rules.make
