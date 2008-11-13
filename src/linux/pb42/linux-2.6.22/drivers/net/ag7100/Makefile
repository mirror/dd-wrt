#
# Makefile for Ar7100 gigebit MAC and Phys
#
obj-$(CONFIG_AG7100)			+=	ag7100_mod.o

obj-phy-$(CONFIG_VITESSE_PHY)	=	vsc8601_phy.o
obj-phy-$(CONFIG_ICPLUS_PHY)	=	ipPhy.o
obj-phy-$(CONFIG_REALTEK_PHY)	=	rtPhy.o
obj-phy-$(CONFIG_ADM6996FC_PHY)	=	adm_phy.o
obj-phy-$(CONFIG_ATHR_PHY) 	=	athrf1_phy.o
obj-phy-$(CONFIG_ATHRS26_PHY) 	=	athrs26_phy.o
obj-phy-$(CONFIG_AG7100_GENERIC_PHY)    = generic_phy.o
obj-phy-$(CONFIG_VITESSE_8601_PHY)      = vsc8601_phy.o
obj-phy-$(CONFIG_VITESSE_8601_7395_PHY) = vsc8601_phy.o generic_i2c.o generic_spi.o vsc73xx.o

ag7100_mod-objs			:=	$(obj-phy-y) ag7100.o

