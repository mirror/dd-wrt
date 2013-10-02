#
# Makefile for the Ralink SoCs built-in ethernet macs
#

ralink-eth-y					+= ralink_soc_eth.o

ralink-eth-$(CONFIG_NET_RALINK_MDIO)		+= mdio.o
ralink-eth-$(CONFIG_NET_RALINK_MDIO_RT2880)	+= mdio_rt2880.o

ralink-eth-$(CONFIG_NET_RALINK_ESW_RT3052)	+= esw_rt3052.o
ralink-eth-$(CONFIG_NET_RALINK_GSW_MT7620)	+= gsw_mt7620a.o

ralink-eth-$(CONFIG_SOC_RT288X_OPENWRT)			+= soc_rt2880.o
ralink-eth-$(CONFIG_SOC_RT305X_OPENWRT)			+= soc_rt305x.o
ralink-eth-$(CONFIG_SOC_RT3883_OPENWRT)			+= soc_rt3883.o
ralink-eth-$(CONFIG_SOC_MT7620_OPENWRT)			+= soc_mt7620.o

obj-$(CONFIG_NET_RALINK)			+= ralink-eth.o
