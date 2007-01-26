/*
 * Copyright (c) 2004 Maxim Sobolev
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: rtp.h,v 1.2 2006/08/11 21:55:44 sobomax Exp $
 *
 */

#ifndef _RTP_H_
#define _RTP_H_

#include <sys/types.h>

/*
 * RTP payload types
 */
typedef enum {
    RTP_PCMU = 0,
    RTP_GSM = 3,
    RTP_G723 = 4,
    RTP_PCMA = 8,
    RTP_CN = 13,
    RTP_G729 = 18,
    RTP_TSE = 100,
    RTP_TSE_CISCO = 101
} rtp_type_t;

/*
 * RTP data header
 */
typedef struct {
#if BYTE_ORDER == BIG_ENDIAN
    unsigned int version:2;	/* protocol version */
    unsigned int p:1;		/* padding flag */
    unsigned int x:1;		/* header extension flag */
    unsigned int cc:4;		/* CSRC count */
    unsigned int m:1;		/* marker bit */
    unsigned int pt:7;		/* payload type */
#else
    unsigned int cc:4;		/* CSRC count */
    unsigned int x:1;		/* header extension flag */
    unsigned int p:1;		/* padding flag */
    unsigned int version:2;	/* protocol version */
    unsigned int pt:7;		/* payload type */
    unsigned int m:1;		/* marker bit */
#endif
    unsigned int seq:16;	/* sequence number */
    uint32_t ts;		/* timestamp */
    uint32_t ssrc;		/* synchronization source */
    uint32_t csrc[0];		/* optional CSRC list */
} rtp_hdr_t;

#define	RTP_HDR_LEN(rhp)	(sizeof(*(rhp)) + ((rhp)->cc * sizeof((rhp)->csrc[0])))

#endif
