#
# Makefile for Ar7100 gigebit MAC and Phys
#
ifndef CONFIG_AG71XX
obj-$(CONFIG_AG7100)			+=	ag7100_mod.o
endif

obj-phy-$(CONFIG_ATHR_VITESSE_PHY)	=	vsc_phy.o
obj-phy-$(CONFIG_ATHR_ICPLUS_PHY)	=	ipPhy.o
#obj-phy-$(CONFIG_NET_DSA)	=	dev-dsa.o
obj-phy-$(CONFIG_ATHR_REALTEK_PHY)	=	rtPhy.o
obj-phy-$(CONFIG_ADM6996FC_PHY)	=	adm_phy.o
ifeq ($(CONFIG_AP81_CARRIER_CUS109), 1)
obj-phy-$(CONFIG_ATHRF1_PHY) 	=	athrf1_phy.o
else
obj-phy-$(CONFIG_ATHR_PHY) 	=	athrf1_phy.o
endif
obj-phy-$(CONFIG_ATHRS26_PHY) 	=	athrs26_phy.o
obj-phy-$(CONFIG_ATHRS16_PHY) 	=	athrs16_phy.o
obj-phy-$(CONFIG_AG7100_GENERIC_PHY)    = generic_phy.o
obj-phy-$(CONFIG_VITESSE_8601_PHY)      = vsc8601_phy.o
obj-phy-$(CONFIG_VITESSE_8601_7395_PHY) = vsc8601_phy.o generic_i2c.o generic_spi.o vsc73xx.o
obj-phy-$(CONFIG_CAMEO_REALTEK_PHY)   = rtl8366sr_phy.o rtl8366_smi.o 

# { append by BUFFALO
RTL8366S_PATH=RTL8366S_DRIVER

obj-phy-$(CONFIG_CAMEO_REALTEK_PHY) += \
	$(RTL8366S_PATH)/rtl8366s_api.o \
	$(RTL8366S_PATH)/rtl8366s_asicdrv.o \
	$(RTL8366S_PATH)/rtl8366s_greenethernet.o 

RTL8366RB_PATH=RTL8366RB_DRIVER
ifeq ($(CONFIG_BUFFALO),y)
obj-phy-$(CONFIG_CAMEO_REALTEK_PHY) += \
	$(RTL8366RB_PATH)/rtl8366rb_api.o \
	$(RTL8366RB_PATH)/rtl8368s_asicdrv.o

obj-phy-$(CONFIG_CAMEO_REALTEK_PHY)     += rtl8366rb_phy.o
endif

#EXTRA_CFLAGS += -DCONFIG_BUFFALO

ifndef CONFIG_AG71XX
ag7100_mod-objs			:=	$(obj-phy-y) ag7100.o
endif

ifeq ($(CONFIG_AP81_CARRIER_CUS109), 1)
# There are two F1e PHY on the CUS109 board.
# One PHY connect to MAC 0, the other one connects to MAC 1.
# Define this symbol to indicate two PHY connect to AR913X.
CFLAGS_athrf1_phy.o += -DCONFIG_CUS109_F1E_PHY
CFLAGS_ag7100.o += -DCONFIG_DUAL_F1E_PHY
endif


