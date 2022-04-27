/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_var.h 1969 2007-01-16 03:05:18Z scottr $
 */
#ifndef _NET80211_IEEE80211_DEBUG_H_
#define _NET80211_IEEE80211_DEBUG_H_

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211.h>

/* Set to true if ANY sc has skb debugging on */
extern int ath_debug_global;
enum {
	GLOBAL_DEBUG_SKB_REF = 0x00000040,	/* SKB reference counting */
	GLOBAL_DEBUG_SKB = 0x40000000,	/* SKB usage/leak debugging,
					   must match ATH_DEBUG_SKB */
};

#define IEEE80211_MSG_NODE_REF  0x80000000	/* node ref counting */
#define	IEEE80211_MSG_DEBUG	0x40000000	/* IFF_DEBUG equivalent */
#define	IEEE80211_MSG_DUMPPKTS	0x20000000	/* IFF_LINK2 equivalent */
#define	IEEE80211_MSG_CRYPTO	0x10000000	/* crypto work */
#define	IEEE80211_MSG_INPUT	0x08000000	/* input handling */
#define	IEEE80211_MSG_XRATE	0x04000000	/* rate set handling */
#define	IEEE80211_MSG_ELEMID	0x02000000	/* element id parsing */
#define	IEEE80211_MSG_NODE	0x01000000	/* node management */
#define	IEEE80211_MSG_ASSOC	0x00800000	/* association handling */
#define	IEEE80211_MSG_AUTH	0x00400000	/* authentication handling */
#define	IEEE80211_MSG_SCAN	0x00200000	/* scanning */
#define	IEEE80211_MSG_OUTPUT	0x00100000	/* output handling */
#define	IEEE80211_MSG_STATE	0x00080000	/* state machine */
#define	IEEE80211_MSG_POWER	0x00040000	/* power save handling */
#define	IEEE80211_MSG_DOT1X	0x00020000	/* 802.1x authenticator */
#define	IEEE80211_MSG_DOT1XSM	0x00010000	/* 802.1x state machine */
#define	IEEE80211_MSG_RADIUS	0x00008000	/* 802.1x radius client */
#define	IEEE80211_MSG_RADDUMP	0x00004000	/* dump 802.1x radius packets */
#define	IEEE80211_MSG_RADKEYS	0x00002000	/* dump 802.1x keys */
#define	IEEE80211_MSG_WPA	0x00001000	/* WPA/RSN protocol */
#define	IEEE80211_MSG_ACL	0x00000800	/* ACL handling */
#define	IEEE80211_MSG_WME	0x00000400	/* WME protocol */
#define	IEEE80211_MSG_SUPG	0x00000200	/* SUPERG */
#define	IEEE80211_MSG_DOTH	0x00000100	/* 11.h */
#define	IEEE80211_MSG_INACT	0x00000080	/* inactivity handling */
#define	IEEE80211_MSG_ROAM	0x00000040	/* sta-mode roaming */
#define	IEEE80211_MSG_CN	0x00000020	/* CN debug information */

#define	IEEE80211_MSG_ANY	0xffffffff	/* anything */

#define IEEE80211_MSG_IC	(IEEE80211_MSG_NODE_REF)	/* shared for all VAP */

#ifdef IEEE80211_DEBUG
#define ieee80211_msg_is_reported(_vap, m) \
	(!!(((_vap)->iv_debug | (_vap)->iv_ic->ic_debug) & (m)))
#define	IEEE80211_DPRINTF(_vap, _m, _fmt, ...) do {			\
	if (ieee80211_msg_is_reported(_vap, _m))					\
		ieee80211_note(_vap, _fmt, __VA_ARGS__);		\
} while (0)
#define	IEEE80211_NOTE(_vap, _m, _ni, _fmt, ...) do {			\
	if (ieee80211_msg_is_reported(_vap, _m))					\
		ieee80211_note_mac(_vap, (_ni)->ni_macaddr, _fmt, __VA_ARGS__);\
} while (0)
#define	IEEE80211_NOTE_MAC(_vap, _m, _mac, _fmt, ...) do {		\
	if (ieee80211_msg_is_reported(_vap, _m))					\
		ieee80211_note_mac(_vap, _mac, _fmt, __VA_ARGS__);	\
} while (0)
#define	IEEE80211_NOTE_FRAME(_vap, _m, _wh, _fmt, ...) do {		\
	if (ieee80211_msg_is_reported(_vap, _m))					\
		ieee80211_note_frame(_vap, _wh, _fmt, __VA_ARGS__);	\
} while (0)
struct ieee80211vap;
struct ieee80211_frame;
void ieee80211_note(struct ieee80211vap *, const char *, ...);
void ieee80211_note_mac(struct ieee80211vap *, const u_int8_t mac[IEEE80211_ADDR_LEN], const char *, ...);
void ieee80211_note_frame(struct ieee80211vap *, const struct ieee80211_frame *, const char *, ...);
#define	ieee80211_msg_debug(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_DEBUG)
#define	ieee80211_msg_dumppkts(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_DUMPPKTS)
#define	ieee80211_msg_input(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_INPUT)
#define	ieee80211_msg_radius(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_RADIUS)
#define	ieee80211_msg_dumpradius(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_RADDUMP)
#define	ieee80211_msg_dumpradkeys(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_RADKEYS)
#define	ieee80211_msg_scan(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_SCAN)
#define	ieee80211_msg_assoc(_vap) \
	ieee80211_msg_is_reported(_vap, IEEE80211_MSG_ASSOC)
#else				/* IEEE80211_DEBUG */
#define	ieee80211_msg_is_reported(_vap, _m)	(0)
#define	IEEE80211_DPRINTF(_vap, _m, _fmt, ...)
#ifndef FORCE_NOTE
#define	IEEE80211_NOTE(_vap, _m, _wh, _fmt, ...)
#else
#define	IEEE80211_NOTE(_vap, _m, _ni, _fmt, ...) do {			\
	printk("%s: " MAC_FMT " - " _fmt "\n", (_vap ? (_vap)->iv_dev->name : "madwifi"), MAC_ADDR((_ni)->ni_macaddr), __VA_ARGS__); \
} while (0)
#endif
#define	IEEE80211_NOTE_FRAME(_vap, _m, _wh, _fmt, ...)
#define	IEEE80211_NOTE_MAC(_vap, _m, _mac, _fmt, ...)
#endif				/* IEEE80211_DEBUG */

#endif				/* _NET80211_IEEE80211_DEBUG_H_ */
