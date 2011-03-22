/*
 * tkhash.h
 * Prototypes for TKIP hash functions.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation; the
 * contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: tkhash.h,v 1.5 2008/12/16 03:56:04 Exp $
 */

#ifndef _TKHASH_H_
#define _TKHASH_H_

#include <typedefs.h>

#define TKHASH_P1_KEY_SIZE	10	/* size of TKHash Phase1 output, in bytes */
#define TKHASH_P2_KEY_SIZE	16	/* size of TKHash Phase2 output */

extern void BCMROMFN(tkhash_phase1)(uint16 *P1K, const uint8 *TK, const uint8 *TA, uint32 IV32);
extern void BCMROMFN(tkhash_phase2)(uint8 *RC4KEY, const uint8 *TK, const uint16 *P1K, uint16 IV16);

#endif /* _TKHASH_H_ */
