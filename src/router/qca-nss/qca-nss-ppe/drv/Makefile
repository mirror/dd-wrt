# Makefile for the PPE drivers

ccflags-y := -I$(obj) -I$(obj)/.. -I$(obj)/../..

ifneq ($(ppe-drv),)
obj-m += ppe_drv/
endif

ifneq ($(ppe-qdisc),)
obj-m += ppe_qdisc/
endif

ifneq ($(ppe-vp),)
obj-m += ppe_vp/
endif

ifneq ($(ppe-tun),)
ifndef PPE_16M_PROFILE
ifndef PPE_256M_PROFILE
obj-m += ppe_tun/
endif
endif
endif

ifneq ($(ppe-ds),)
obj-m += ppe_ds/
endif

ifneq ($(ppe-rule),)
obj-m += ppe_rule/
endif

obj ?= .
