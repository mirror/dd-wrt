/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
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
 * $Id: rtp.h,v 1.11 2009/01/12 11:36:40 sobomax Exp $
 *
 */

#ifndef _RTP_H_
#define _RTP_H_

#include <sys/socket.h>
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

#define RTP_NSAMPLES_UNKNOWN  (-1)

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

typedef struct {
    uint16_t profile;		/* defined by profile */
    uint16_t length;		/* length of the following array in 32-byte words */
    uint32_t extension[0];	/* actual extension data */
} rtp_hdr_ext_t;

struct rtp_packet {
    size_t      size;

    struct sockaddr_storage raddr;
    struct sockaddr *laddr;

    socklen_t   rlen;
    size_t      data_size;
    int         data_offset;
    int         nsamples;
    uint32_t    ts;
    uint16_t    seq;
    int         appendable;
    double      rtime;
    int         rport;

    struct rtp_packet *next;
    struct rtp_packet *prev;

    /*
     * The packet, keep it the last member so that we can use
     * memcpy() only on portion that it's actually being
     * utilized.
     */
    union {
	rtp_hdr_t       header;
	unsigned char   buf[8192];
    } data;
};

struct rtp_packet_chunk {
    int bytes;
    int nsamples;
    int whole_packet_matched;
};

typedef enum {
    RTP_PARSER_OK = 0,
    RTP_PARSER_PTOOSHRT = -1,
    RTP_PARSER_IHDRVER = -2,
    RTP_PARSER_PTOOSHRTXS = -3,
    RTP_PARSER_PTOOSHRTXH = -4,
    RTP_PARSER_PTOOSHRTPS = -5,
    RTP_PARSER_PTOOSHRTP = -6,
    RTP_PARSER_IPS = -7
} rtp_parser_err_t;

#define	RTP_HDR_LEN(rhp)	(sizeof(*(rhp)) + ((rhp)->cc * sizeof((rhp)->csrc[0])))

const char *rtp_packet_parse_errstr(rtp_parser_err_t);
rtp_parser_err_t rtp_packet_parse(struct rtp_packet *);
struct rtp_packet *rtp_recv(int);

struct rtp_packet *rtp_packet_alloc();
void rtp_packet_free(struct rtp_packet *);
void rtp_packet_set_seq(struct rtp_packet *, uint16_t seq);
void rtp_packet_set_ts(struct rtp_packet *, uint32_t ts);

void rtp_packet_first_chunk_find(struct rtp_packet *, struct rtp_packet_chunk *, int min_nsamples);

#define ts_less(ts1, ts2) (((ts1) - (ts2)) > (uint32_t) (1 << 31))

#endif
