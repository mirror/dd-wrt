LOC_DIR=src/ref
LIB=REF

include $(PRJ_PATH)/make/config.mk

SRC_LIST_TMP=ref_uci.c
#SRC_SW_LIST=ref_misc.c ref_uci.c
SRC_SW_LIST=ref_misc.c ref_uci.c ref_athtag.c ref_vport.c ref_mapt.c ref_tunnel.c

ifeq (TRUE, $(IN_FDB))
	SRC_LIST_TMP += ref_fdb.c
endif

ifeq (TRUE, $(IN_MIB))
	SRC_LIST_TMP += ref_mib.c
endif

ifeq (TRUE, $(IN_MISC))
	SRC_LIST_TMP += ref_misc.c
endif

ifeq (TRUE, $(IN_PORTCONTROL))
	SRC_LIST_TMP += ref_port_ctrl.c
endif

ifeq (TRUE, $(IN_VLAN))
	SRC_LIST_TMP += ref_vlan.c
endif

ifeq (TRUE, $(IN_VSI))
	SRC_LIST_TMP += ref_vsi.c
endif

ifeq (TRUE, $(IN_TUNNEL))
	SRC_LIST_TMP += ref_tunnel.c
endif

ifeq (TRUE, $(IN_MAPT))
	SRC_LIST_TMP += ref_mapt.c
endif

ifeq (TRUE, $(IN_VPORT))
	SRC_LIST_TMP += ref_vport.c
endif

ifeq (TRUE, $(IN_ACL))
	SRC_LIST_TMP += ref_acl.c
endif

ifeq (TRUE, $(IN_ATHTAG))
	SRC_LIST_TMP += ref_athtag.c
endif

ifeq (FALSE, $(SWCONFIG))
	SRC_LIST=$(filter-out $(SRC_SW_LIST), $(SRC_LIST_TMP))
else
	SRC_LIST=$(SRC_LIST_TMP)
endif


include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj

