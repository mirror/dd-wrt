LOC_DIR=src/adpt/mppe
LIB=ADPT

include $(PRJ_PATH)/make/config.mk

SRC_LIST=

ifeq (TRUE, $(IN_SERVCODE))
  SRC_LIST += adpt_mppe_servcode.c
endif

ifeq (TRUE, $(IN_ATHTAG))
  SRC_LIST += adpt_mppe_athtag.c
endif

ifeq (, $(filter MPPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
