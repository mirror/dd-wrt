# Helper makefile for building Broadcom wl device driver
# This file maps wl driver feature flags (import) to WLFLAGS and WLFILES (export).
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
# $Id: wl.mk,v 1.1 2005/09/01 13:19:08 seg Exp $

# os-independent config flag -> WLFLAGS and WLFILES mapping

# debug/internal
ifeq ($(DEBUG),1)
WLFLAGS += -DBCMDBG
endif

## wl driver common 
#w/wpa
ifeq ($(WL),1)
WLFILES += wlc.c d11ucode.c wlc_phy.c wlc_rate.c
WLFILES += wlc_security.c rc4.c tkhash.c bcmwpa.c
endif

## wl OSL
ifeq ($(WLVX),1)
WLFILES += wl_vx.c
endif

ifeq ($(WLLX),1)
WLFILES += wl_linux.c
endif

ifeq ($(WLLXIW),1)
WLFILES += wl_iw.c
endif

ifeq ($(WLNDIS),1)
WLFILES += wl_ndis.c
WLFILES += wl_ndconfig.c
endif

ifeq ($(WLCFE),1)
WLFILES += wl_cfe.c
endif

ifeq ($(WLRTE),1)
WLFILES += wl_rte.c
endif

## wl special
# oids

#ifdef BINOSL
ifeq ($(BINOSL),1)
WLFLAGS += -DBINOSL
endif
#endif

## wl features
# ap
ifeq ($(AP),1)
WLFLAGS += -DAP
endif

# sta
ifeq ($(STA),1)
WLFLAGS += -DSTA
endif

# wet
ifeq ($(WET),1)
WLFLAGS += -DWET
WLFILES += wlc_wet.c
endif

# led
ifeq ($(WLLED),1)
WLFLAGS += -DWLLED
WLFILES += wlc_led.c
endif

# WME
ifeq ($(WME),1)
WLFLAGS += -DWME
endif

# PIO
ifeq ($(PIO),1)
WLFLAGS += -DPIO
endif

# CRAM
ifeq ($(CRAM),1)
WLFLAGS += -DCRAM
endif

# 11H 
ifeq ($(WL11H),1)
WLFLAGS += -DWL11H
endif

# 11D 
ifeq ($(WL11D),1)
WLFLAGS += -DWL11D
endif

# DBAND
ifeq ($(DBAND),1)
WLFLAGS += -DDBAND
endif

# WLRM
ifeq ($(WLRM),1)
WLFLAGS += -DWLRM
endif

# WLCQ
ifeq ($(WLCQ),1)
WLFLAGS += -DWLCQ
endif

## wl security
# in-driver supplicant
ifeq ($(BCMSUP_PSK),1)
WLFLAGS += -DBCMSUP_PSK
WLFILES += wlc_sup.c aes.c md5.c rijndael-alg-fst.c aeskeywrap.c hmac.c passhash.c prf.c sha1.c
endif

# bcmccx

# BCMWPA2
ifeq ($(BCMWPA2),1)
WLFLAGS += -DBCMWPA2
#WLFILES += aes.c aeskeywrap.c
endif

## wl over jtag
#ifdef BCMJTAG
ifeq ($(BCMJTAG),1)
  WLFLAGS += -DBCMJTAG
  WLFILES += bcmjtag.c ejtag.c jtagm.c
endif
#endif


## --- which buses

# silicon backplane

ifeq ($(BCMSBBUS),1)
WLFLAGS += -DBCMBUSTYPE=SB_BUS
endif


# sdio

## --- basic shared files

ifeq ($(HNDDMA),1)
WLFILES += hnddma.c
endif

ifeq ($(BCMUTILS),1)
WLFILES += bcmutils.c
endif

ifeq ($(BCMSROM),1)
WLFILES += bcmsrom.c
endif

ifeq ($(SBUTILS),1)
WLFILES += sbutils.c
endif

ifeq ($(SBMIPS),1)
WLFILES += sbmips.c
endif

ifeq ($(SBSDRAM),1)
WLFILES += sbsdram.c
endif

ifeq ($(SBPCI),1)
WLFILES += sbpci.c
endif

ifeq ($(SFLASH),1)
WLFILES += sflash.c
endif

ifeq ($(FLASHUTL),1)
WLFILES += flashutl.c
endif


## --- shared OSL
# linux osl
ifeq ($(OSLLX),1)
WLFILES += linux_osl.c
endif

ifeq ($(OSLLXPCI),1)
WLFILES += linux_pci.c
endif

# vx osl
ifeq ($(OSLVX),1)
WLFILES += vx_osl.c
endif

ifeq ($(OSLCFE),1)
WLFILES += cfe_osl.c
endif

ifeq ($(OSLRTE),1)
WLFILES += hndrte_osl.c
endif

ifeq ($(OSLNDIS),1)
WLFILES += ndshared.c ndis_osl.c
endif

ifeq ($(CONFIG_USBRNDIS_RETAIL),1)
WLFLAGS += -DCONFIG_USBRNDIS_RETAIL
WLFILES += wl_ndconfig.c
endif

ifeq ($(NVRAM),1)
WLFILES += nvram.c
endif

ifeq ($(NVRAMVX),1)
WLFILES += nvram_vx.c
endif

ifeq ($(NVRAMRO),1)
WLFILES += nvram_ro.c sflash.c
endif

#wlinfo:
#	@echo "WLFLAGS=\"$(WLFLAGS)\""
#	@echo "WLFILES=\"$(WLFILES)\""
