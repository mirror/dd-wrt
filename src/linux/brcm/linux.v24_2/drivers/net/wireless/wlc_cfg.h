/*
 * Configuration-related definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2005-2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 *                                     
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef _wlc_cfg_h_
#define _wlc_cfg_h_

/**************************************************
 * Get customized tunables to override the default*
 * ************************************************
 */
#include "wlconf.h"

/***********************************************
 * Feature-related macros to optimize out code *
 * *********************************************
 */

/* Some useful combinations */
#define STA_ONLY(wlc) 1

/* Check if a particular BSS config is AP or STA */
/* NOTE: References structure defined in wlc.h */
#define BSSCFG_STA(bsscfg)		(1)

/* NOTE: References structure fields defined in wlc.h */
#define STA_ACTIVE(wlc)	((wlc)->sta_associated || ((wlc)->assoc_state != AS_IDLE))

/* DUALBAND Support */
#ifdef DBAND
#define NBANDS(wlc) ((wlc)->pub._nbands)
#define NBANDS_PUB(pub) ((pub)->_nbands)
#else
#define NBANDS(wlc) 1
#define NBANDS_PUB(wlc) 1
#endif /* DBAND */

/* WME Support */
#define WME_ENAB(wlc) 0

/* PIO Mode Support */
#ifdef WLPIO
#define PIO_ENAB(wlc) ((wlc)->_piomode)
#else
#define PIO_ENAB(wlc) 0
#endif /* WLPIO */

/* Spectrum Management -- 11H Support */
#define WL11H_ENAB(wlc) 0

/********************************************************************
 * Phy/Core Configuration.  Defines macros to to check core phy/rev *
 * compile-time configuration.  Defines default core support.       *
 * ******************************************************************
 */

/* Basic macros to check a configuration bitmask */

#define CONF_HAS(config, val)	((config) & (1 << (val)))
#define CONF_MSK(config, mask)	((config) & (mask))
#define MSK_RANGE(low, hi)	((1 << ((hi)+1)) - (1 << (low)))
#define CONF_RANGE(config, low, hi) (CONF_MSK(config, MSK_RANGE(low, high)))

#define CONF_IS(config, val)	((config) == (1 << (val)))
#define CONF_GE(config, val)	((config) & (0-(1 << (val))))
#define CONF_GT(config, val)	((config) & (0-2*(1 << (val))))
#define CONF_LT(config, val)	((config) & ((1 << (val))-1))
#define CONF_LE(config, val)	((config) & (2*(1 << (val))-1))

/* Wrappers for some of the above, specific to config constants */

#define ACONF_HAS(val)	CONF_HAS(ACONF, val)
#define ACONF_MSK(mask)	CONF_MSK(ACONF, mask)
#define ACONF_IS(val)	CONF_IS(ACONF, val)
#define ACONF_GE(val)	CONF_GE(ACONF, val)
#define ACONF_GT(val)	CONF_GT(ACONF, val)
#define ACONF_LT(val)	CONF_LT(ACONF, val)
#define ACONF_LE(val)	CONF_LE(ACONF, val)

#define GCONF_HAS(val)	CONF_HAS(GCONF, val)
#define GCONF_MSK(mask)	CONF_MSK(GCONF, mask)
#define GCONF_IS(val)	CONF_IS(GCONF, val)
#define GCONF_GE(val)	CONF_GE(GCONF, val)
#define GCONF_GT(val)	CONF_GT(GCONF, val)
#define GCONF_LT(val)	CONF_LT(GCONF, val)
#define GCONF_LE(val)	CONF_LE(GCONF, val)

#define D11CONF_HAS(val) CONF_HAS(D11CONF, val)
#define D11CONF_MSK(mask) CONF_MSK(D11CONF, mask)
#define D11CONF_IS(val)	CONF_IS(D11CONF, val)
#define D11CONF_GE(val)	CONF_GE(D11CONF, val)
#define D11CONF_GT(val)	CONF_GT(D11CONF, val)
#define D11CONF_LT(val)	CONF_LT(D11CONF, val)
#define D11CONF_LE(val)	CONF_LE(D11CONF, val)

#define PHYCONF_HAS(val) CONF_HAS(PHYTYPE, val)
#define PHYCONF_IS(val)	CONF_IS(PHYTYPE, val)

/* Macros to check (but override) a run-time value; compile-time
 * override allows unconfigured code to be optimized out.
 *
 * NOTE: includes compile-time check for forced 0 AND forced 1
 * NOTE: single bit/value arg works for small zero-based enums only
 */

#define AREV_IS(var, val)	(ACONF_HAS(val) && (ACONF_IS(val) || ((var) == (val))))
#define AREV_GE(var, val)	(ACONF_GE(val) && (!ACONF_LT(val) || ((var) >= (val))))
#define AREV_GT(var, val)	(ACONF_GT(val) && (!ACONF_LE(val) || ((var) > (val))))
#define AREV_LT(var, val)	(ACONF_LT(val) && (!ACONF_GE(val) || ((var) < (val))))
#define AREV_LE(var, val)	(ACONF_LE(val) && (!ACONF_GT(val) || ((var) <= (val))))

#define GREV_IS(var, val)	(GCONF_HAS(val) && (GCONF_IS(val) || ((var) == (val))))
#define GREV_GE(var, val)	(GCONF_GE(val) && (!GCONF_LT(val) || ((var) >= (val))))
#define GREV_GT(var, val)	(GCONF_GT(val) && (!GCONF_LE(val) || ((var) > (val))))
#define GREV_LT(var, val)	(GCONF_LT(val) && (!GCONF_GE(val) || ((var) < (val))))
#define GREV_LE(var, val)	(GCONF_LE(val) && (!GCONF_GT(val) || ((var) <= (val))))

#define D11REV_IS(var, val)	(D11CONF_HAS(val) && (D11CONF_IS(val) || ((var) == (val))))
#define D11REV_GE(var, val)	(D11CONF_GE(val) && (!D11CONF_LT(val) || ((var) >= (val))))
#define D11REV_GT(var, val)	(D11CONF_GT(val) && (!D11CONF_LE(val) || ((var) > (val))))
#define D11REV_LT(var, val)	(D11CONF_LT(val) && (!D11CONF_GE(val) || ((var) < (val))))
#define D11REV_LE(var, val)	(D11CONF_LE(val) && (!D11CONF_GT(val) || ((var) <= (val))))

#define PHYTYPE_IS(var, val)	(PHYCONF_HAS(val) && (PHYCONF_IS(val) || ((var) == (val))))

/* Finally, early-exit from switch case if anyone wants it... */

#define CASECHECK(config, val)	if (!(CONF_HAS(config, val))) break
#define CASEMSK(config, mask)	if (!(CONF_MSK(config, mask))) break

/* **** Core type/rev defaults **** */

#define D11_DEFAULT	0x0fb0	/* Supported  D11 revs: 4, 5, 7-11 */
#define APHY_DEFAULT	0x00ec	/* Supported aphy revs:
				 *	2	4306b0
				 *	3	4306c0, 4712a0/a1/a2/b0
				 *	5	4320a2
				 *	6	4318b0
				 *	7	5352a0, 4311a0
				 */
#define GPHY_DEFAULT	0x01c6	/* Supported gphy revs:
				 *	1	4306b0
				 *	2	4306c0, 4712a0/a1/a2/b0
				 *	6	4320a2
				 *	7	4318b0, 5352a0
				 *	8	4311a0
				 */
/* For undefined values, use defaults */
#ifndef D11CONF
#define D11CONF D11_DEFAULT
#endif /* D11CONF */
#ifndef ACONF
#define ACONF APHY_DEFAULT
#endif /* ACONF */
#ifndef GCONF
#define GCONF GPHY_DEFAULT
#endif /* GCONF */

#if (D11CONF ^ (D11CONF & D11_DEFAULT))
#error "Unsupported MAC revision configured"
#endif /* (D11CONF ^ (D11CONF & D11_DEFAULT)) */
#if (ACONF ^ (ACONF & APHY_DEFAULT))
#error "Unsupported APHY revision configured"
#endif /* (ACONF ^ (ACONF & APHY_DEFAULT)) */
#if (GCONF ^ (GCONF & GPHY_DEFAULT))
#error "Unsupported GPHY revision configured"
#endif /* (GCONF ^ (GCONF & GPHY_DEFAULT)) */

/* *** Consistency checks *** */

#if !D11CONF
#error "No MAC revisions configured!"
#endif /* !D11CONF */
#if !ACONF && !GCONF
#error "No PHY configured!"
#endif /* !ACONF && !GCONF */

/* Set up PHYTYPE automatically: (depends on PHY_TYPE_X, from d11.h) */
#if ACONF
#define _PHYCONF_A (1 << PHY_TYPE_A)
#else
#define _PHYCONF_A 0
#endif /* ACONF */

#if GCONF
#define _PHYCONF_G (1 << PHY_TYPE_G)
#else
#define _PHYCONF_G 0
#endif /* GCONF */

#define PHYTYPE (_PHYCONF_A | _PHYCONF_G)

/* Last but not least: shorter wlc-specific var checks */
#define ISAPHY(pi)	PHYTYPE_IS((pi)->phy_type, PHY_TYPE_A)
#define ISBPHY(pi)	PHYTYPE_IS((pi)->phy_type, PHY_TYPE_B)
#define ISGPHY(pi)	PHYTYPE_IS((pi)->phy_type, PHY_TYPE_G)

/**********************************************************************
 * ------------- End of Core phy/rev configuration. ----------------- *
 * ********************************************************************
 */

/*************************************************
 * Defaults for tunables (e.g. sizing constants) *
 * ***********************************************
 */
#ifndef NTXD
#define NTXD		256   /* Max # of entries in Tx FIFO based on 4kb page size */
#endif /* NTXD */
#ifndef NRXD
#define NRXD		256   /* Max # of entries in Rx FIFO based on 4kb page size */
#endif /* NRXD */

#ifndef NRXBUFPOST
#define	NRXBUFPOST	16		/* try to keep this # rbufs posted to the chip */
#endif /* NRXBUFPOST */

#ifndef MAXSCB				    /* station control blocks in cache */
#define MAXSCB		32		/* Maximum SCBs in cache for STA */
#endif /* MAXSCB */

#ifndef MAXBSS
#define MAXBSS		64	/* max # available networks */
#endif /* MAXBSS */

#ifndef WLC_DATAHIWAT
#define WLC_DATAHIWAT		50	/* data msg txq hiwat mark */
#endif /* WLC_DATAHIWAT */

#define	MAXCHANNEL		224	/* max # supported channels. The max channel no is 216,
					 * this is that + 1 rounded up to a multiple of NBBY (8).
					 * DO NOT MAKE it > 255: channels are uint8's all over
					 */
#ifdef WLCNT
#define WLC_UPDATE_STATS(wlc)	1	/* Stats support */
#define WLCNTINCR(a)		((a)++)	/* Increment by 1 */
#define WLCNTDECR(a)		((a)--)	/* Decrement by 1 */
#define WLCNTADD(a,delta)	((a) += (delta)) /* Increment by specified value */
#define WLCNTSET(a,value)	((a) = (value)) /* Set to specific value */
#define WLCNTVAL(a)		(a)	/* Return value */
#else /* WLCNT */
#define WLC_UPDATE_STATS(wlc)	0	/* No stats support */
#define WLCNTINCR(a)			/* No stats support */
#define WLCNTDECR(a)			/* No stats support */
#define WLCNTADD(a,delta)		/* No stats support */
#define WLCNTSET(a,value)		/* No stats support */
#define WLCNTVAL(a)		0	/* No stats support */
#endif /* WLCNT */

/* bounded rx loops */
#ifndef RXBND
#define RXBND		12	/* max # frames to process in wlc_recv() */
#endif	/* RXBND */
#ifndef TXSBND
#define TXSBND		8	/* max # tx status to process in wlc_txstatus() */
#endif	/* TXSBND */

#if defined(ACONF)
#define BAND_5G(bt)	((bt) == WLC_BAND_5G)
#else
#define BAND_5G(bt)	0
#endif

#if defined(GCONF)
#define BAND_2G(bt)	((bt) == WLC_BAND_2G)
#else
#define BAND_2G(bt)	0
#endif

/* Some phy initialization code/data can't be reclaimed in dualband mode */
#ifdef DBAND
#define WLBANDINITDATA(_data)	_data
#define WLBANDINITFN(_fn)		_fn
#else
#define WLBANDINITDATA(_data)	BCMINITDATA(_data)
#define WLBANDINITFN(_fn)		BCMINITFN(_fn)
#endif

#endif /* _wlc_cfg_h_ */
