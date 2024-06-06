LOC_DIR=src/init
LIB=INIT

include $(PRJ_PATH)/make/config.mk

SRC_LIST=ssdk_init.c ssdk_plat.c ssdk_interrupt.c ssdk_dts.c ssdk_phy_i2c.c

ifneq (ISISC, $(CHIP_TYPE))
	SRC_LIST += ssdk_clk.c
endif

ifeq (TRUE, $(IN_LED))
	SRC_LIST += ssdk_led.c
endif

ifneq (,$(findstring HPPE, $(SUPPORT_CHIP)))
	SRC_LIST += ssdk_hppe.c
endif

ifneq (,$(findstring SCOMPHY, $(SUPPORT_CHIP)))
	SRC_LIST += ssdk_scomphy.c
endif

ifneq (,$(filter MP, $(SUPPORT_CHIP)))
	SRC_LIST += ssdk_mp.c
endif

ifneq (,$(filter APPE, $(SUPPORT_CHIP)))
	SRC_LIST += ssdk_appe.c
endif

ifneq (,$(filter MHT, $(SUPPORT_CHIP)))
	SRC_LIST += ssdk_mht.c ssdk_mht_clk.c ssdk_mht_pinctrl.c
endif

ifeq (TRUE, $(IN_NETLINK))
	SRC_LIST += ssdk_netlink.c
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
