#
# defaults
#
ifndef ATH_CAP_SUPERG_FF
export ATH_CAP_SUPERG_FF=1
endif

ifndef ATH_CAP_DYNTURBO
export ATH_CAP_DYNTURBO=1
endif

ifndef ATH_CAP_SUPERG_COMP
export ATH_CAP_SUPERG_COMP=1
endif

ifndef ATH_CAP_XR
export ATH_CAP_XR=1
endif

ifndef ATH_CAP_TPC
export ATH_CAP_TPC=0
endif

ifndef ATH_CAP_TX99
export ATH_CAP_TX99=0
endif

#
# directives
#
ifneq ($(strip ${ATH_CAP_SUPERG_FF}),0)
COPTS2+= -DATH_SUPERG_FF=1
endif

ifneq ($(strip ${ATH_CAP_DYNTURBO}),0)
COPTS2+= -DATH_SUPERG_DYNTURBO=1 -DATH_TURBO_SCAN=1
endif

ifneq ($(strip ${ATH_CAP_SUPERG_COMP}),0)
COPTS2+= -DATH_SUPERG_COMP=1
endif

ifneq ($(strip ${ATH_CAP_XR}),0)
COPTS2+= -DATH_SUPERG_XR=1
endif

ifneq ($(strip ${ATH_CAP_TPC}),0)
COPTS2+= -DATH_CAP_TPC=1
endif

ifneq ($(strip ${ATH_CAP_TX99}),0)
COPTS2+= -DATH_TX99_DIAG=1
endif
