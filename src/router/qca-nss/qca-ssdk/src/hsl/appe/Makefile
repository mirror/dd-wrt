LOC_DIR=src/hsl/appe
LIB=HSL

include $(PRJ_PATH)/make/config.mk

SRC_LIST=appe_global.c appe_l2_vp.c appe_counter.c

ifeq (TRUE, $(IN_PORTVLAN))
	SRC_LIST+=appe_portvlan.c
endif

ifeq (TRUE, $(IN_VSI))
	SRC_LIST+=appe_vsi.c
endif

ifeq (TRUE, $(IN_TUNNEL))
	SRC_LIST+=appe_tunnel.c
endif

ifeq (TRUE, $(IN_MAPT))
	SRC_LIST+=appe_tunnel_map.c
endif

ifeq (TRUE, $(IN_PPPOE))
	SRC_LIST+=appe_pppoe.c
endif

ifeq (TRUE, $(IN_ACL))
	SRC_LIST+=appe_acl.c
endif

ifeq (TRUE, $(IN_VXLAN))
	SRC_LIST+=appe_vxlan.c
endif

ifeq (TRUE, $(IN_GENEVE))
	SRC_LIST+=appe_geneve.c
endif

ifeq (TRUE, $(IN_TUNNEL_PROGRAM))
	SRC_LIST+=appe_tunnel_program.c
endif

ifeq (TRUE, $(IN_QM))
	SRC_LIST+=appe_qm.c
endif

ifeq (TRUE, $(IN_SHAPER))
	SRC_LIST+=appe_shaper.c
endif

ifeq (TRUE, $(IN_POLICER))
	SRC_LIST+=appe_policer.c
endif

ifeq (TRUE, $(IN_SERVCODE))
	SRC_LIST+=appe_servcode.c
endif

ifeq (TRUE, $(IN_SEC))
	SRC_LIST+=appe_sec.c
endif

ifeq (, $(filter APPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj

