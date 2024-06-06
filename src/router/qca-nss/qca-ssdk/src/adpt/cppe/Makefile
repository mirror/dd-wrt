LOC_DIR=src/adpt/cppe
LIB=ADPT

include $(PRJ_PATH)/make/config.mk

SRC_LIST=

ifeq (TRUE, $(IN_PORTCONTROL))
  SRC_LIST += adpt_cppe_portctrl.c
endif

ifeq (TRUE, $(IN_QOS))
  SRC_LIST += adpt_cppe_qos.c
endif

ifeq (TRUE, $(IN_UNIPHY))
  SRC_LIST += adpt_cppe_uniphy.c
endif

ifeq (TRUE, $(IN_FLOW))
  SRC_LIST += adpt_cppe_flow.c
endif

ifeq (TRUE, $(IN_MIB))
  SRC_LIST += adpt_cppe_mib.c
endif

ifeq (, $(findstring CPPE, $(SUPPORT_CHIP)))
ifneq (, $(filter APPE, $(SUPPORT_CHIP)))
	SRC_LIST= adpt_cppe_qos.c adpt_cppe_flow.c
else
	SRC_LIST=
endif
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
