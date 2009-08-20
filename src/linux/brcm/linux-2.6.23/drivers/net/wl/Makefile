#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.9 2008/08/11 19:38:26 Exp $
#

SRCBASE := ../../../brcm
EXTRA_CFLAGS+=-I$(TOPDIR)/brcm/include -DCONFIG_NOPRINTK=1

TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

# Generate list of supported variants
VLIST_BASE := AP STA APSTA

ifeq ($(CONFIG_WL_USE_HIGH),y)
VLIST += HIGH
else
VLIST          += $(VLIST_BASE)
ifeq ($(BUILD_WL_ONCHIP_G),true)
  VLIST        += $(foreach base,$(VLIST_BASE),$(base)_ONCHIP_G)
endif # BUILD_WL_ONCHIP_G
endif


# Set suffix and config based on the selected variant
WL_SUFFIX := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))
CONFIG_WL_CONF := $(CONFIG_WL_$(WL_SUFFIX))

# Module file names use lowercase of the suffix
wl_suffix := $(shell echo "$(WL_SUFFIX)" | tr A-Z a-z)


# For PREBUILD and postbuild w/WLSRC, compile variants in subdirs
# Subdir names use lowercase versions of the variant suffixes
wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)/" | tr A-Z a-z)))


obj-y += $(wlsubdirs)


include $(src)/wl_generic.mk
