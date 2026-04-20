LOC_DIR=src/hsl/mht
LIB=HSL

include $(PRJ_PATH)/make/config.mk

SRC_LIST= mht_init.c mht_sec_ctrl.c

ifeq (TRUE, $(IN_IP))
	SRC_LIST += mht_ip.c
endif

ifeq (TRUE, $(IN_COSMAP))
	SRC_LIST += mht_cosmap.c
endif

ifeq (TRUE, $(IN_NAT))
	SRC_LIST += mht_nat.c
endif

ifeq (TRUE, $(IN_PORTCONTROL))
	SRC_LIST += mht_port_ctrl.c
endif

ifeq (TRUE, $(IN_INTERFACECONTROL))
	SRC_LIST += mht_interface_ctrl.c
endif

ifeq (, $(findstring MHT, $(SUPPORT_CHIP)))
	SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
