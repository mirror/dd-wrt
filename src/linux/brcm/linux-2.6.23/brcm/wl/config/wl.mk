# Helper makefile for building Broadcom wl device driver
# This file maps wl driver feature flags (import) to WLFLAGS and WLFILES_SRC (export).
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
# $Id: wl.mk,v 1.210.2.52 2008/10/22 02:28:08 Exp $


# debug/internal
ifeq ($(DEBUG),1)
	WLFLAGS += -DBCMDBG -DWLTEST
else
	# This is true for mfgtest builds.
#ifdef WLTEST
	ifeq ($(WLTEST),1)
		WLFLAGS += -DWLTEST
	endif
#endif
endif


#ifdef BCMDBG_MEM
ifeq ($(BCMDBG_MEM),1)
	WLFLAGS += -DBCMDBG_MEM
endif
#endif

#ifdef BCMDBG_PKT
ifeq ($(BCMDBG_PKT),1)
	WLFLAGS += -DBCMDBG_PKT
endif
#endif

#ifdef WLLMAC
ifeq ($(WLLMAC),1)
	WLFLAGS += -DWLLMAC -DEXTENDED_SCAN
endif
#endif


## wl driver common

ifndef WL_LOW
WL_LOW = 1
endif

ifndef WL_HIGH
WL_HIGH = 1
endif

## iff one of WLC_LOW and WLC_HIGH is defined, SPLIT is true
WL_SPLIT = 0
ifeq ($(WL_LOW),1)
	ifneq ($(WL_HIGH),1)
		WL_SPLIT = 1
	endif
endif
ifeq ($(WL_HIGH),1)
	ifneq ($(WL_LOW),1)
		WL_SPLIT = 1
	endif
endif

ifeq ($(WL_LOW),1)
	WLFLAGS += -DWLC_LOW
endif

ifeq ($(WL_HIGH),1)
	WLFLAGS += -DWLC_HIGH
endif

# split driver infrastructure files
ifeq ($(WL_SPLIT),1)
	WLFILES_SRC += src/shared/bcm_xdr.c
	WLFILES_SRC += src/shared/bcm_rpc.c
	WLFILES_SRC_HI += src/shared/nvramstubs.c
	ifeq ($(OSLLX),1)
		WLFILES_SRC_HI += src/shared/linux_rpc_osl.c
	endif
	ifeq ($(OSLNDIS),1)
		WLFILES_SRC_HI += src/shared/ndis_rpc_osl.c
	endif
	ifeq ($(BCMDBUS),1)
		WLFILES_SRC_HI += src/shared/bcm_rpc_tp_dbus.c
	endif

	WLFILES_SRC_HI += src/wl/sys/wlc_bmac_stubs.c
	WLFILES_SRC_HI += src/wl/sys/wlc_rpctx.c
	WLFILES_SRC_LO += src/wl/sys/wlc_high_stubs.c

	ifeq ($(WL_HIGH),1)
		WLFLAGS += -DBCMBUSTYPE=RPC_BUS
		WLFLAGS += -DOSLREGOPS
	endif
endif

#ifdef WL
ifeq ($(WL),1)
	WLFILES_SRC += src/shared/bcmwifi.c

	WLFILES_SRC_LO += src/wl/sys/wlc_bmac.c
	WLFILES_SRC_LO += src/wl/sys/d11ucode.c
	WLFILES_SRC_LO += src/wl/sys/wlc_phy.c
	WLFILES_SRC_LO += src/shared/qmath.c
	WLFILES_SRC_LO += src/wl/sys/mimophytbls.c
	WLFILES_SRC_LO += src/wl/sys/lpphytbls.c
	WLFILES_SRC_LO += src/wl/sys/sslpnphytbls.c

	WLFILES_SRC_HI += src/wl/sys/wlc.c
	WLFILES_SRC_HI += src/wl/sys/wlc_rate.c
	WLFILES_SRC_HI += src/wl/sys/wlc_security.c
	WLFILES_SRC_HI += src/wl/sys/wlc_key.c
	WLFILES_SRC_HI += src/wl/sys/wlc_scb.c
	WLFILES_SRC_HI += src/wl/sys/wlc_rate_sel.c
	WLFILES_SRC_HI += src/wl/sys/wlc_antsel.c
	WLFILES_SRC_HI += src/wl/sys/wlc_bsscfg.c
	WLFILES_SRC_HI += src/wl/sys/wlc_scan.c
	WLFILES_SRC_HI += src/wl/sys/wlc_phy_iovar.c
	ifeq ($(WLLMAC),1)
		WLFILES_SRC += src/wl/sys/wlc_lmac.c
		ifeq ($(STA), 1)
			WLFILES_SRC += src/wl/sys/wlc_lmac_sta.c
		endif
		ifeq ($(WLLMACPROTO), 1)
			WLFLAGS += -DWLLMACPROTO
			WLFILES_SRC += src/wl/sys/wlc_lmac_proto.c
		endif
	else
		WLFILES_SRC_HI += src/wl/sys/wlc_event.c
		WLFILES_SRC_HI += src/wl/sys/wlc_channel.c
	endif
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES_SRC_HI += src/shared/bcmwpa.c
#ifndef LINUX_CRYPTO		   
	ifneq ($(LINUX_CRYPTO),1)
		WLFILES_SRC_HI += src/bcmcrypto/rc4.c
		WLFILES_SRC_HI += src/bcmcrypto/tkhash.c
		WLFILES_SRC_HI += src/bcmcrypto/tkmic.c
		WLFILES_SRC_HI += src/bcmcrypto/wep.c
	endif
#endif
	endif
endif
#endif

#ifdef BCMDBUS
ifeq ($(BCMDBUS),1)
	WLFLAGS += -DBCMDBUS
	WLFILES_SRC += src/shared/dbus.c

	ifeq ($(BCMSDIO),1)
		WLFILES_SRC += src/shared/dbus_sdio.c
	else
		WLFILES_SRC += src/shared/dbus_usb.c
	endif

	ifeq ($(WLLX),1)
		ifeq ($(BCMSDIO),1)
			WLFILES_SRC += src/shared/dbus_sdio_linux.c
		else
			WLFILES_SRC += src/shared/dbus_usb_linux.c
		endif
	else
		ifeq ($(WLNDIS),1)
			ifeq ($(BCMSDIO),1)
				WLFILES_SRC += src/shared/dbus_sdio_ndis.c
			else
				WLFILES_SRC += src/shared/dbus_usb_ndis.c
			endif
		endif
	endif
endif
#endif

#ifndef LINUX_HYBRID
ifeq ($(BCM_DNGL_EMBEDIMAGE),1)
	WLFLAGS += -DBCM_DNGL_EMBEDIMAGE
endif
#endif

## wl OSL
#ifdef WLVX
ifeq ($(WLVX),1)
	WLFILES_SRC += src/wl/sys/wl_vx.c
	WLFILES_SRC += src/shared/bcmstdlib.c
	WLFLAGS += -DSEC_TXC_ENABLED
endif
#endif

#ifdef WLBSD
ifeq ($(WLBSD),1)
	WLFILES_SRC += src/wl/sys/wl_bsd.c
endif
#endif

#ifdef WLLX
ifeq ($(WLLX),1)
	ifneq ($(WL_HIGH),1)
		WLFILES_SRC_LO += src/wl/sys/wl_linux_bmac.c
		WLFILES_SRC_LO += src/shared/bcm_rpc_char.c
	endif
	WLFILES_SRC_HI += src/wl/sys/wl_linux.c
endif
#endif

#ifdef WLLXIW
ifeq ($(WLLXIW),1)
	WLFILES_SRC_HI += src/wl/sys/wl_iw.c
endif
#endif

#ifdef WLCFE
ifeq ($(WLCFE),1)
	WLFILES_SRC += src/wl/sys/wl_cfe.c
endif
#endif

#ifdef WLRTE
ifeq ($(WLRTE),1)
	WLFILES_SRC += src/wl/sys/wl_rte.c
	ifneq ($(WL_HIGH),1)
		WLFILES_SRC_LO += src/shared/bcm_rpc_tp_rte.c
	endif
endif
#endif

ifeq ($(BCMECICOEX),1)
	WLFLAGS += -DBCMECICOEX
endif


#ifdef NDIS
# anything Windows/NDIS specific for 2k/xp/vista/windows7
ifeq ($(WLNDIS),1)

	WLFILES_SRC += src/wl/sys/wl_ndis.c

	ifeq ($(WLNDIS_DHD),)
		WLFILES_SRC += src/wl/sys/nhd_ndis.c
	else
		WLFILES_SRC += src/dhd/sys/dhd_ndis.c
	endif
	WLFLAGS += -DMEMORY_TAG="'7034'"
	WLFILES_SRC += src/wl/sys/wl_ndconfig.c

	WLFILES_SRC += src/shared/bcmwifi.c
	WLFILES_SRC += src/shared/bcmstdlib.c

	# support host supplied nvram variables
	ifeq ($(WLTEST),1)
		ifeq ($(WLHOSTVARS), 1)
			WLFLAGS += -DBCMHOSTVARS
		endif
	endif

	ifeq ($(WLVISTA),1)
		WLFLAGS += -DEXT_STA

		WLFLAGS += -DWLNOEIND
		WLFLAGS += -DWL_MONITOR
	        WLFLAGS += -DIBSS_PEER_GROUP_KEY
	        WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
	        WLFLAGS += -DIBSS_PEER_MGMT
	endif
	ifeq ($(WLXP),1)
		WLFLAGS += -DWLNOEIND
	endif

	# HIGH driver for BMAC ?? any ndis/xp/vista ?
	ifeq ($(WL_SPLIT),1)

	endif

	# DHD host: ?? to clean up and to support all other DHD OSes
	ifeq ($(WLNDIS_DHD),1)
		WLFLAGS += -DSHOW_EVENTS -DBCMPERFSTATS
		WLFLAGS += -DBDC -DBCMDONGLEHOST	
		WLFLAGS += -DBCM4325 
		ifeq ($(BCMSDIO),)
			WLFLAGS += -DBCMDHDUSB
			WLFLAGS += -DBCM4328 -DBCM4322
		endif

		WLFILES_SRC += src/dhd/sys/dhd_cdc.c
		WLFILES_SRC += src/dhd/sys/dhd_common.c

		BCMPCI=0

		ifeq ($(BCMDBUS),)
			WLFILES_SRC += src/dhd/sys/dhd_usb_ndis.c
		endif	

		ifeq ($(WLVISTA),1)
			WLFILES_SRC += src/wl/sys/wlc_rate.c
		endif

		ifeq ($(WLXP),1)
			WLFLAGS += -DNDIS_DMAWAR 
			# move these non-wl flag to makefiles
			WLFLAGS += -DBINARY_COMPATIBLE -DWIN32_LEAN_AND_MEAN=1 
		endif		
	endif
endif
#endif


#ifdef BINOSL
ifeq ($(BINOSL),1)
	WLFLAGS += -DBINOSL
endif
#endif

## wl features
# NCONF -- 0 is remove from code, else bit mask of supported nphy revs
ifneq ($(NCONF),)
	WLFLAGS += -DNCONF=$(NCONF)
endif

# ACONF -- 0 is remove from code, else bit mask of supported aphy revs
ifneq ($(ACONF),)
	WLFLAGS += -DACONF=$(ACONF)
endif

# GCONF -- 0 is remove from code, else bit mask of supported gphy revs
ifneq ($(GCONF),)
	WLFLAGS += -DGCONF=$(GCONF)
endif

# LPCONF -- 0 is remove from code, else bit mask of supported lpphy revs
ifneq ($(LPCONF),)
	WLFLAGS += -DLPCONF=$(LPCONF)
endif


#ifdef AP
# ap
ifeq ($(AP),1)
	WLFILES_SRC_HI += src/wl/sys/wlc_ap.c
	WLFILES_SRC_HI += src/wl/sys/wlc_apps.c
	WLFILES_SRC_HI += src/wl/sys/wlc_apcs.c
	WLFLAGS += -DAP

	ifeq ($(MBSS),1)
		WLFLAGS += -DMBSS
	endif

	# WME_PER_AC_TX_PARAMS
	ifeq ($(WME_PER_AC_TX_PARAMS),1)
		WLFLAGS += -DWME_PER_AC_TX_PARAMS
	endif

	# WME_PER_AC_TUNING
	ifeq ($(WME_PER_AC_TUNING),1)
		WLFLAGS += -DWME_PER_AC_TUNING
	endif

endif
#endif

#ifdef STA
# sta
ifeq ($(STA),1)
	WLFLAGS += -DSTA
endif
#endif

#ifdef APSTA
# apsta
ifeq ($(APSTA),1)
	WLFLAGS += -DAPSTA
endif
# apsta
#endif

#ifdef WET
# wet
ifeq ($(WET),1)
	WLFLAGS += -DWET
	WLFILES_SRC_HI += src/wl/sys/wlc_wet.c
endif
#endif

#ifdef WMF
ifeq ($(WMF), 1)
	WLFILES_SRC_HI += src/wl/sys/wlc_wmf.c
	WLFLAGS += -DWMF
endif 
#endif

#ifdef MAC_SPOOF
# mac spoof
ifeq ($(MAC_SPOOF),1)
	WLFLAGS += -DMAC_SPOOF
endif
#endif

#ifndef LINUX_HYBRID
# Router IBSS Security Support
ifeq ($(ROUTER_SECURE_IBSS),1)
         WLFLAGS += -DIBSS_PEER_GROUP_KEY
         WLFLAGS += -DIBSS_PSK
         WLFLAGS += -DIBSS_PEER_MGMT
         WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
endif
#endif

#ifdef WLLED
# led
ifeq ($(WLLED),1)
	WLFLAGS += -DWLLED
	WLFILES_SRC_HI += src/wl/sys/wlc_led.c
endif
#endif
      
#ifdef WL_MONITOR
# MONITOR
ifeq ($(WL_MONITOR),1)
	WLFLAGS += -DWL_MONITOR
endif
#endif

#ifdef ND_ALL_PASSIVE
ifeq ($(ND_ALL_PASSIVE),1)
	WLFLAGS += -DND_ALL_PASSIVE
endif
#endif

#ifdef WME
# WME
ifeq ($(WME),1)
	WLFLAGS += -DWME
	ifeq ($(WMMAC), 1)
		ifeq ($(WL), 1)
			WLFLAGS += -DWLCAC
			WLFILES_SRC_HI += src/wl/sys/wlc_cac.c
		endif
	endif
endif
#endif

#ifdef WMMAC_FLOWCONTROL
# WMMAC section
ifeq ($(WMMAC), 1)
	WLFLAGS += -DWMMAC
endif
#endif

#ifdef WLBA
# WLBA
ifeq ($(WLBA),1)
	WLFLAGS += -DWLBA
	WLFILES_SRC_HI += src/wl/sys/wlc_ba.c
endif
#endif

#ifdef WLPIO
# WLPIO 
ifeq ($(WLPIO),1)
	WLFLAGS += -DWLPIO
	WLFILES_SRC_LO += src/wl/sys/wlc_pio.c
endif
#endif

#ifdef WLAFTERBURNER
# WLAFTERBURNER
ifeq ($(WLAFTERBURNER),1)
	WLFLAGS += -DWLAFTERBURNER
endif
#endif

#ifdef CRAM
# CRAM
ifeq ($(CRAM),1)
	WLFLAGS += -DCRAM
	WLFILES_SRC_HI += src/wl/sys/wlc_cram.c
endif
#endif

#ifdef WL11N
# 11N 
ifeq ($(WL11N),1)
	WLFLAGS += -DWL11N
endif
#endif

#ifdef WL11H
# 11H 
ifeq ($(WL11H),1)
	WLFLAGS += -DWL11H
endif
#endif

#ifdef WL11D
# 11D 
ifeq ($(WL11D),1)
	WLFLAGS += -DWL11D
endif
#endif

#ifdef DBAND
# DBAND
ifeq ($(DBAND),1)
	WLFLAGS += -DDBAND
endif
#endif
      
#ifdef WLRM
# WLRM
ifeq ($(WLRM),1)
	WLFLAGS += -DWLRM
endif
#endif
      
#ifdef WLCQ
# WLCQ
ifeq ($(WLCQ),1)
	WLFLAGS += -DWLCQ
endif
#endif

#ifdef WLCNT
# WLCNT
ifeq ($(WLCNT),1)
	WLFLAGS += -DWLCNT
endif
#endif

ifndef DELTASTATS
	ifeq ($(WLCNT),1)
		DELTASTATS := 1
	endif
endif

# DELTASTATS
ifeq ($(DELTASTATS),1)
	WLFLAGS += -DDELTASTATS
endif

#ifdef WLCNTSCB
# WLCNTSCB
ifeq ($(WLCNTSCB),1)
	WLFLAGS += -DWLCNTSCB
endif
#endif


#ifdef WLCOEX
# WLCOEX
ifeq ($(WLCOEX),1)
	WLFLAGS += -DWLCOEX
endif
#endif

## wl security
# external linux supplicant
#ifdef LINUX_CRYPTO
ifeq ($(LINUX_CRYPTO), 1)
	WLFLAGS += -DLINUX_CRYPTO
endif
#endif

#ifdef BCMSUP_PSK
# in-driver supplicant
ifeq ($(BCMSUP_PSK),1)
	WLFLAGS += -DBCMSUP_PSK
	WLFILES_SRC_HI += src/wl/sys/wlc_sup.c
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES_SRC_HI += src/bcmcrypto/aes.c
		WLFILES_SRC_HI += src/bcmcrypto/aeskeywrap.c
		WLFILES_SRC_HI += src/bcmcrypto/hmac.c
		WLFILES_SRC_HI += src/bcmcrypto/prf.c
		WLFILES_SRC_HI += src/bcmcrypto/sha1.c
		##NetBSD 2.0 has MD5 and AES built in
		ifneq ($(OSLBSD),1)
			WLFILES_SRC_HI += src/bcmcrypto/md5.c
			WLFILES_SRC_HI += src/bcmcrypto/rijndael-alg-fst.c
		endif
	endif
	WLFILES_SRC_HI += src/bcmcrypto/passhash.c
endif
#endif


#ifdef BCMAUTH_PSK
# in-driver authenticator
ifeq ($(BCMAUTH_PSK),1)
	WLFLAGS += -DBCMAUTH_PSK
	WLFILES_SRC_HI += src/wl/sys/wlc_auth.c
endif
#endif

#ifdef BCMWPA2
# BCMWPA2
ifeq ($(BCMWPA2),1)
	WLFLAGS += -DBCMWPA2
endif
#endif

#ifdef BCMCCMP
# Soft AES CCMP
ifeq ($(BCMCCMP),1)
	WLFLAGS += -DBCMCCMP
	ifneq ($(BCMROMOFFLOAD),1)
		WLFILES_SRC_HI += src/bcmcrypto/aes.c
		##BSD has  AES built in
		ifneq ($(BSD),1)
			WLFILES_SRC_HI +=src/bcmcrypto/rijndael-alg-fst.c
		endif
	endif
endif
#endif


# BCMDMA64
ifeq ($(BCMDMA64),1)
	WLFLAGS += -DBCMDMA64
endif

ifeq ($(BCMDMA64OSL),1)
	WLFLAGS += -DBCMDMA64OSL
endif

## wl over jtag
#ifdef BCMJTAG
ifeq ($(BCMJTAG),1)
	WLFLAGS += -DBCMJTAG -DBCMSLTGT
	WLFILES_SRC += src/shared/bcmjtag.c
	WLFILES_SRC += src/shared/bcmjtag_linux.c
	WLFILES_SRC += src/shared/ejtag.c
	WLFILES_SRC += src/shared/jtagm.c
endif
#endif

#ifdef WLAMSDU
ifeq ($(WLAMSDU),1)
	WLFLAGS += -DWLAMSDU
	WLFILES_SRC_HI += src/wl/sys/wlc_amsdu.c
endif
#endif

#ifdef WLAMSDU_SWDEAGG
ifeq ($(WLAMSDU_SWDEAGG),1)
	WLFLAGS += -DWLAMSDU_SWDEAGG
endif
#endif

#ifdef WLAMPDU
ifeq ($(WLAMPDU),1)
	WLFLAGS += -DWLAMPDU
	WLFILES_SRC_HI += src/wl/sys/wlc_ampdu.c
endif
#endif

#ifdef WOWL
ifeq ($(WOWL),1)
	WLFLAGS += -DWOWL
	WLFILES_SRC += src/wl/sys/d11wakeucode.c
	WLFILES_SRC += src/wl/sys/wlc_wowl.c
	WLFILES_SRC += src/wl/sys/wowlaestbls.c
endif
#endif

#ifdef BTC2WIRE
ifeq ($(BTC2WIRE),1)
	WLFLAGS += -DBTC2WIRE
	WLFILES_SRC_LO += src/wl/sys/d11ucode_2w.c
endif
#endif

#ifdef WL_ASSOC_RECREATE
ifeq ($(WL_ASSOC_RECREATE),1)
	ifeq ($(STA),1)
		WLFLAGS += -DWL_ASSOC_RECREATE
	endif
endif
#endif


#ifdef WLBTAMP
ifeq ($(WLBTAMP),1)
	WLFLAGS += -DWLBTAMP
	WLFILES_SRC_HI += src/wl/sys/wlc_bta.c
endif
#endif

#ifdef WLPLT
ifeq ($(WLPLT),1)
	WLFLAGS += -DWLPLT
	WLFILES_SRC_HI += src/wl/sys/wlc_plt.c
endif
#endif


## --- which buses

# silicon backplane

#ifdef BCMSIBUS
ifeq ($(BCMSIBUS),1)
	WLFLAGS += -DBCMBUSTYPE=SI_BUS
	BCMPCI=0
endif
#endif

ifeq ($(SOCI_SB),1)
	WLFLAGS += -DBCMCHIPTYPE=SOCI_SB
else 
	ifeq ($(SOCI_AI),1)
		WLFLAGS += -DBCMCHIPTYPE=SOCI_AI
	endif
endif



#ifndef LINUX_HYBRID
# AP/ROUTER with SDSTD
ifeq ($(WLAPSDSTD),1)
	WLFILES_SRC += src/shared/nvramstubs.c
	WLFILES_SRC += src/shared/bcmsrom.c
endif
#endif

## --- basic shared files

#ifdef HNDDMA
ifeq ($(HNDDMA),1)
	WLFILES_SRC_LO += src/shared/hnddma.c
endif
#endif

#ifdef BCMUTILS
ifeq ($(BCMUTILS),1)
	WLFILES_SRC += src/shared/bcmutils.c
endif
#endif

#ifdef BCMSROM
ifeq ($(BCMSROM),1)
	WLFILES_SRC_LO += src/shared/bcmsrom.c
	WLFILES_SRC_LO += src/shared/bcmotp.c
endif
#endif

#ifdef SIUTILS
ifeq ($(SIUTILS),1)
	WLFILES_SRC_LO += src/shared/siutils.c
	WLFILES_SRC_LO += src/shared/sbutils.c
	WLFILES_SRC_LO += src/shared/aiutils.c
	WLFILES_SRC_LO += src/shared/hndpmu.c
	ifneq ($(BCMPCI), 0)
		WLFILES_SRC_LO += src/shared/nicpci.c
	endif
endif
#endif

#ifdef SBMIPS
ifeq ($(SBMIPS),1)
	WLFLAGS += -DBCMMIPS
	WLFILES_SRC_LO += src/shared/hndmips.c
	WLFILES_SRC_LO += src/shared/hndchipc.c
endif
#endif

#ifdef SBPCI
ifeq ($(SBPCI),1)
	WLFILES_SRC_LO += src/shared/hndpci.c
endif
#endif

#ifdef SFLASH
ifeq ($(SFLASH),1)
	WLFILES_SRC_LO += src/shared/sflash.c
endif
#endif

#ifdef FLASHUTL
ifeq ($(FLASHUTL),1)
	WLFILES_SRC_LO += src/shared/flashutl.c
endif
#endif

## --- shared OSL
#ifdef OSLLX
# linux osl
ifeq ($(OSLLX),1)
	WLFILES_SRC += src/shared/linux_osl.c
endif
#endif

#ifdef OSLVX
# vx osl
ifeq ($(OSLVX),1)
	WLFILES_SRC += src/shared/vx_osl.c
	WLFILES_SRC += src/shared/bcmallocache.c
endif
#endif

#ifdef OSLBSD
# bsd osl
ifeq ($(OSLBSD),1)
	WLFILES_SRC += src/shared/bsd_osl.c
	WLFILES_SRC += src/shared/nvramstubs.c
endif
#endif

#ifdef OSLCFE
ifeq ($(OSLCFE),1)
	WLFILES_SRC += src/shared/cfe_osl.c
endif
#endif

#ifdef OSLRTE
ifeq ($(OSLRTE),1)
	WLFILES_SRC += src/shared/hndrte_osl.c
endif
#endif

#ifdef OSLNDIS
ifeq ($(OSLNDIS),1)
	WLFILES_SRC += src/shared/ndshared.c
	WLFILES_SRC += src/shared/ndis_osl.c
endif
#endif

#ifndef LINUX_HYBRID
ifeq ($(CONFIG_USBRNDIS_RETAIL),1)
	WLFLAGS += -DCONFIG_USBRNDIS_RETAIL
	WLFILES_SRC += src/wl/sys/wl_ndconfig.c
	WLFILES_SRC += src/shared/bcmwifi.c
endif

ifeq ($(NVRAM),1)
	WLFILES_SRC_LO += src/dongle/rte/test/nvram.c
	WLFILES_SRC_LO += src/dongle/rte/sim/nvram.c
	WLFILES_SRC_LO += src/shared/nvram/nvram.c
endif

ifeq ($(NVRAMVX),1)
	WLFILES_SRC_LO += src/shared/nvram/nvram_rw.c
endif
#endif

#ifdef BCMNVRAMR
ifeq ($(BCMNVRAMR),1)
	WLFILES_SRC_LO += src/shared/nvram/nvram_ro.c
	WLFILES_SRC_LO += src/shared/sflash.c
	WLFILES_SRC_LO += src/shared/bcmotp.c
	WLFLAGS += -DBCMNVRAMR
endif
#else
ifneq ($(BCMNVRAMR),1)
	ifeq ($(WLLXNOMIPSEL),1)
		WLFILES_SRC += src/shared/nvramstubs.c
	else
		ifeq ($(WLNDIS),1)
			WLFILES_SRC += src/shared/nvramstubs.c
		else
			ifeq ($(BCMNVRAMW),1)
				WLFILES_SRC_LO += src/shared/nvram/nvram_ro.c
				WLFILES_SRC_LO += src/shared/sflash.c
			endif
		endif
	endif
	ifeq ($(BCMNVRAMW),1)
		WLFILES_SRC_LO += src/shared/bcmotp.c
		WLFLAGS += -DBCMNVRAMW
	endif
endif
#endif

# Auto-select OTP: BCMAUTOOTP defined
# IPX OTP: nothing defined
# HND OTP: BCMHNDOTP defined
ifeq ($(BCMAUTOOTP),1)
	WLFLAGS += -DBCMAUTOOTP
endif
ifeq ($(BCMHNDOTP),1)
	WLFLAGS += -DBCMHNDOTP
endif


#ifdef WLDIAG
ifeq ($(WLDIAG),1)
	WLFLAGS += -DWLDIAG
	WLFILES_SRC_LO += src/wl/sys/wlc_diag.c
endif
#endif

#ifdef BCMDBG
ifneq ($(BCMDBG),1)
	ifeq ($(WLTINYDUMP),1)
		WLFLAGS += -DWLTINYDUMP
	endif
endif
#endif

#ifdef BCMQT
ifeq ($(BCMQT),1)
  # Set flag to indicate emulated chip
  WLFLAGS += -DBCMSLTGT -DBCMQT
  ifeq ($(WLRTE),1)
    # Use of RTE implies embedded (CPU emulated)
    WLFLAGS += -DBCMQT_CPU
  endif
endif
#endif

#ifdef WLPFN
ifeq ($(WLPFN),1)
	WLFLAGS += -DWLPFN
	WLFILES_SRC += src/wl/sys/wl_pfn.c
endif
#endif

#ifdef TOE
ifeq ($(TOE),1)
	WLFLAGS += -DTOE
	WLFILES_SRC += src/wl/sys/wl_toe.c
endif
#endif

#ifdef ARPOE
ifeq ($(ARPOE),1)
	WLFLAGS += -DARPOE
	WLFILES_SRC += src/wl/sys/wl_arpoe.c
endif
#endif

#ifdef PCOEM_LINUXSTA
ifeq ($(PCOEM_LINUXSTA),1)
	WLFLAGS += -DPCOEM_LINUXSTA
endif
#endif

#ifdef LINUXSTA_PS
ifeq ($(LINUXSTA_PS),1)
	WLFLAGS += -DLINUXSTA_PS
endif
#endif

#ifndef LINUX_HYBRID
ifeq ($(KEEP_ALIVE),1)
	WLFLAGS += -DKEEP_ALIVE
	WLFILES_SRC += src/wl/sys/wl_keep_alive.c
endif

#ifdef OPENSRC_IOV_IOCTL
ifeq ($(OPENSRC_IOV_IOCTL),1)
	WLFLAGS += -DOPENSRC_IOV_IOCTL
endif
#endif

ifeq ($(PACKET_FILTER),1)
	WLFLAGS += -DPACKET_FILTER
	WLFILES_SRC += src/wl/sys/wlc_pkt_filter.c
endif

ifeq ($(SEQ_CMDS),1)
	WLFLAGS += -DSEQ_CMDS
	WLFILES_SRC += src/wl/sys/wlc_seq_cmds.c
endif

ifeq ($(RECEIVE_THROTTLE),1)
	WLFLAGS += -DWL_PM2_RCV_DUR_LIMIT
endif

ifeq ($(ASYNC_TSTAMPED_LOGS),1)
	WLFLAGS += -DBCMTSTAMPEDLOGS
endif
#endif


# Sort and remove duplicates from WLFILES* 
ifeq ($(WL_LOW),1)
	WLFILES_SRC += $(sort $(WLFILES_SRC_LO))
endif
ifeq ($(WL_HIGH),1)
	WLFILES_SRC += $(sort $(WLFILES_SRC_HI))
endif


# Legacy WLFILES pathless definition, please use new src relative path
# in make files. 
WLFILES := $(sort $(notdir $(WLFILES_SRC)))
