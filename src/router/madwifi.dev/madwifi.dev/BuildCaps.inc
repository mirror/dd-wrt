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
export ATH_CAP_SUPERG_COMP=0
endif

ifndef HAVE_POLLING
export HAVE_POLLING=0
endif

ifndef ATH_CAP_XR
export ATH_CAP_XR=1
endif

ifndef ATH_CAP_TPC
export ATH_CAP_TPC=1
endif

ifndef ATH_CAP_TX99
export ATH_CAP_TX99=0
endif

# WARNING: The use of these extensions may introduce a security risk (someone can peek/poke registers on your Atheros device)
ifndef ATH_REVERSE_ENGINEERING
export ATH_REVERSE_ENGINEERING=0
endif
ifndef ATH_REVERSE_ENGINEERING_WITH_NO_FEAR 
export ATH_REVERSE_ENGINEERING_WITH_NO_FEAR=0
endif

#
# directives
#
ifneq ($(strip $(ATH_CAP_SUPERG_FF)),0)
COPTS+= -DATH_SUPERG_FF=1
endif

ifneq ($(strip $(ATH_CAP_DYNTURBO)),0)
COPTS+= -DATH_SUPERG_DYNTURBO=1 -DATH_TURBO_SCAN=1
endif

ifneq ($(strip $(ATH_CAP_SUPERG_COMP)),0)
COPTS+= -DATH_SUPERG_COMP=1
endif

ifneq ($(strip $(HAVE_POLLING)),0)
COPTS+= -DHAVE_POLLING=1
endif

ifneq ($(strip $(ATH_CAP_XR)),0)
COPTS+= -DATH_SUPERG_XR=1
endif

ifneq ($(strip $(ATH_CAP_TPC)),0)
COPTS+= -DATH_CAP_TPC=1
endif

ifneq ($(strip $(ATH_CAP_TX99)),0)
COPTS+= -DATH_TX99_DIAG=1
endif

# WARNING: The use of these extensions may introduce a security risk (someone can peek/poke registers on your Atheros device)
ifneq ($(strip $(ATH_REVERSE_ENGINEERING)),0)
COPTS+= -DATH_REVERSE_ENGINEERING=1
endif
ifneq ($(strip $(ATH_REVERSE_ENGINEERING_WITH_NO_FEAR)),0)
COPTS+= -DATH_REVERSE_ENGINEERING_WITH_NO_FEAR=1 -DATH_REVERSE_ENGINEERING=1 
endif
#COPTS+= -DHAVE_WPROBE
export SINGLE_MODULE=1
