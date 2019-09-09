# SPDX-License-Identifier: ISC
#
# Makefile fragment for Broadcom 802.11 Networking Device Driver
#
# Copyright (c) 2010 Broadcom Corporation
#

ccflags-y += \
	-I$(backport_srctree)/drivers/net/wireless/broadcom/brcm80211/brcmfmac	\
	-I$(backport_srctree)/drivers/net/wireless/broadcom/brcm80211/include

obj-$(CPTCFG_BRCMFMAC) += brcmfmac.o
brcmfmac-objs += \
		cfg80211.o \
		chip.o \
		fwil.o \
		fweh.o \
		fwsignal.o \
		p2p.o \
		proto.o \
		common.o \
		core.o \
		firmware.o \
		feature.o \
		btcoex.o \
		vendor.o \
		pno.o
brcmfmac-$(CPTCFG_BRCMFMAC_PROTO_BCDC) += \
		bcdc.o
brcmfmac-$(CPTCFG_BRCMFMAC_PROTO_MSGBUF) += \
		commonring.o \
		flowring.o \
		msgbuf.o
brcmfmac-$(CPTCFG_BRCMFMAC_SDIO) += \
		sdio.o \
		bcmsdh.o
brcmfmac-$(CPTCFG_BRCMFMAC_USB) += \
		usb.o
brcmfmac-$(CPTCFG_BRCMFMAC_PCIE) += \
		pcie.o
brcmfmac-y += \
		debug.o
brcmfmac-$(CPTCFG_BRCM_TRACING) += \
		tracepoint.o
brcmfmac-$(CONFIG_OF) += \
		of.o
brcmfmac-$(CONFIG_DMI) += \
		dmi.o
