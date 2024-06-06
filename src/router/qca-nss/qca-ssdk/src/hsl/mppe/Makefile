LOC_DIR=src/hsl/mppe
LIB=HSL

include $(PRJ_PATH)/make/config.mk

ifeq (TRUE, $(IN_ATHTAG))
  SRC_LIST+=mppe_athtag.c
endif

ifeq (TRUE, $(IN_SERVCODE))
  SRC_LIST+=mppe_servcode.c
endif

ifeq (, $(filter MPPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
