/*
 * Broadcom 802.11abg Networking Device Driver Configuration file
 *
 * Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: wltunable_lx_router.h,v 1.10.2.4 2011-01-05 20:42:55 Exp $
 *
 * wl driver tunables
 */

#define D11CONF		0x77a74000	/* D11 Core Rev 5 (4306C0), 7 (4712), 9 (4318b0, 5352), */
//#define D11CONF		0x77a7baa0

#define NRXBUFPOST	128 	/* # rx buffers posted */
#define RXBND		24	/* max # rx frames to process */
#define CTFPOOLSZ       192	/* max buffers in ctfpool */

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1
