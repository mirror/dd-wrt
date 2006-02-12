#
# Makefile for the Broadcom wl driver
#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.2 2005/03/29 03:32:18 mbm Exp $

EXTRA_CFLAGS += -I$(TOPDIR)/arch/mips/bcm947xx/include

O_TARGET	:= wl.o

obj-y		:= apsta_aeskeywrap.o apsta_aes.o apsta_bcmwpa.o apsta_d11ucode.o
obj-y		+= apsta_hmac.o apsta_md5.o apsta_passhash.o apsta_prf.o apsta_rc4.o
obj-y		+= apsta_rijndael-alg-fst.o apsta_sha1.o apsta_tkhash.o apsta_wlc_led.o
obj-y		+= apsta_wlc_phy.o apsta_wlc_rate.o apsta_wlc_security.o 
obj-y		+= apsta_wlc_sup.o apsta_wlc_wet.o apsta_wl_linux.o apsta_wlc.o

obj-m		:= $(O_TARGET)

include $(TOPDIR)/Rules.make


clean: 
