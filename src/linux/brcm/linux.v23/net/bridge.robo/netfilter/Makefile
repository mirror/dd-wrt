#
# Makefile for the netfilter modules on top of bridging.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET	:= netfilter.o

export-objs := ebtables.o

obj-$(CONFIG_BRIDGE_NF_EBTABLES) += ebtables.o
obj-$(CONFIG_BRIDGE_EBT_T_FILTER) += ebtable_filter.o
obj-$(CONFIG_BRIDGE_EBT_T_NAT) += ebtable_nat.o
obj-$(CONFIG_BRIDGE_EBT_BROUTE) += ebtable_broute.o
obj-$(CONFIG_BRIDGE_EBT_802_3) += ebt_802_3.o
obj-$(CONFIG_BRIDGE_EBT_ARPF) += ebt_arp.o
obj-$(CONFIG_BRIDGE_EBT_AMONG) += ebt_among.o
obj-$(CONFIG_BRIDGE_EBT_IPF) += ebt_ip.o
obj-$(CONFIG_BRIDGE_EBT_LIMIT) += ebt_limit.o
obj-$(CONFIG_BRIDGE_EBT_MARKF) += ebt_mark_m.o
obj-$(CONFIG_BRIDGE_EBT_PKTTYPE) += ebt_pkttype.o
obj-$(CONFIG_BRIDGE_EBT_STP) += ebt_stp.o
obj-$(CONFIG_BRIDGE_EBT_VLANF) += ebt_vlan.o
obj-$(CONFIG_BRIDGE_EBT_LOG) += ebt_log.o
obj-$(CONFIG_BRIDGE_EBT_LOG) += ebt_ulog.o
obj-$(CONFIG_BRIDGE_EBT_ARPREPLY) += ebt_arpreply.o
obj-$(CONFIG_BRIDGE_EBT_DNAT) += ebt_dnat.o
obj-$(CONFIG_BRIDGE_EBT_MARK_T) += ebt_mark.o
obj-$(CONFIG_BRIDGE_EBT_REDIRECT) += ebt_redirect.o
obj-$(CONFIG_BRIDGE_EBT_SNAT) += ebt_snat.o
include $(TOPDIR)/Rules.make
