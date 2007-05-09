/*
 * Minimal debug/trace/assert driver definitions for
 * Broadcom 802.11 Networking Adapter.
 *
 *
 * Copyright 2005-2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef _wl_dbg_
#define _wl_dbg_

/* wl_msg_level is a bitvector with defs in wlioctl.h */

#define	WL_ERROR(args)
#define	WL_TRACE(args)
#define	WL_PRHDRS(i, s, h, p, n, l)
#define	WL_PRPKT(m, b, n)
#define	WL_INFORM(args)
#define	WL_TMP(args)
#define WL_OID(args)
#define	WL_RATE(args)
#define	WL_ASSOC(args)
#define	WL_PRUSR(m, b, n)
#define WL_PS(args)
#define WL_TXPWR(args)
#define WL_GMODE(args)
#define WL_DUAL(args)
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#define WL_NRSSI(args)
#define WL_LOFT(args)
#define WL_REGULATORY(args)
#define WL_ACI(args)
#define WL_RADAR(args)
#define WL_MPC(args)
#define WL_BA(args)
#define WL_NITRO(args)
#define	WL_PHYDBG(args)
#define	WL_PHYCAL(args)
#define	WL_CAC(args)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_DFS(args)

/* To disable a message completely ... until you need it again */
#define	WL_NONE(args)

#define WL_ERROR_ON()		0
#define WL_PRHDRS_ON()		0
#define WL_PRPKT_ON()		0
#define WL_INFORM_ON()		0
#define WL_OID_ON()		0
#define WL_ASSOC_ON()		0
#define WL_WSEC_ON()		0
#define WL_WSEC_DUMP_ON()	0
#define WL_MPC_ON()		0
#define WL_REGULATORY_ON()	0
#define WL_BA_ON()		0
#define WL_NITRO_ON()		0
#define WL_DFS_ON()		0

extern uint wl_msg_level;

#endif /* _wl_dbg_ */
