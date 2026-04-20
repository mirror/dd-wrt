LOC_DIR=src/adpt/appe
LIB=ADPT

include $(PRJ_PATH)/make/config.mk

SRC_LIST=

ifeq (TRUE, $(IN_FDB))
  SRC_LIST += adpt_appe_fdb.c
endif

ifeq (TRUE, $(IN_PORTCONTROL))
  SRC_LIST += adpt_appe_portctrl.c
endif

ifeq (TRUE, $(IN_VSI))
  SRC_LIST += adpt_appe_vsi.c
endif

ifeq (TRUE, $(IN_VPORT))
  SRC_LIST += adpt_appe_vport.c
endif

ifeq (TRUE, $(IN_PPPOE))
  SRC_LIST += adpt_appe_pppoe.c
endif

ifeq (TRUE, $(IN_TUNNEL))
  SRC_LIST += adpt_appe_tunnel.c
endif

ifeq (TRUE, $(IN_VXLAN))
  SRC_LIST += adpt_appe_vxlan.c
endif

ifeq (TRUE, $(IN_GENEVE))
  SRC_LIST += adpt_appe_geneve.c
endif

ifeq (TRUE, $(IN_TUNNEL_PROGRAM))
  SRC_LIST += adpt_appe_tunnel_program.c
endif

ifeq (TRUE, $(IN_MAPT))
  SRC_LIST += adpt_appe_mapt.c
endif

ifeq (TRUE, $(IN_QM))
  SRC_LIST += adpt_appe_qm.c
endif

ifeq (TRUE, $(IN_CTRLPKT))
  SRC_LIST += adpt_appe_ctrlpkt.c
endif

ifeq (TRUE, $(IN_SHAPER))
  SRC_LIST += adpt_appe_shaper.c
endif

ifeq (TRUE, $(IN_POLICER))
  SRC_LIST += adpt_appe_policer.c
endif

ifeq (TRUE, $(IN_ACL))
  SRC_LIST += adpt_appe_acl.c
endif

ifeq (TRUE, $(IN_SERVCODE))
  SRC_LIST += adpt_appe_servcode.c
endif

ifeq (TRUE, $(IN_SEC))
  SRC_LIST += adpt_appe_sec.c
endif

ifeq (TRUE, $(IN_PORTVLAN))
  SRC_LIST += adpt_appe_portvlan.c
endif

ifeq (, $(filter APPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
