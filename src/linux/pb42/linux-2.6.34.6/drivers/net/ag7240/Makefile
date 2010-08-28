#
# Makefile for Ar7240 gigabit MAC and Phys
#
obj-$(CONFIG_AG7240)			+=	ag7240_mod.o

obj-phy-$(CONFIG_AR7240_S26_PHY) 	+= ar7240_s26_phy.o
obj-phy-$(CONFIG_AR7240_S26_VLAN_IGMP)		+= python_vlan_igmp.o

ag7240_mod-objs			:=	$(obj-phy-y) ag7240.o

