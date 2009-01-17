/*
 * Copyright (c) 2007-2008 Sippy Software, Inc., http://www.sippysoft.com
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
 * $Id: rtp.c,v 1.9 2009/01/12 11:36:40 sobomax Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>

#include "rtp.h"
#include "rtpp_util.h"

/* Linked list of free packets */
static struct rtp_packet *rtp_packet_pool = NULL;

static int 
g723_len(unsigned char ch)
{

    switch (ch & 3) {
    case 2:
	/* Silence Insertion Descriptor (SID) frame */
	return 4;

    case 0:
	/* 6.3 kbit/s frame */
	return 24;

    case 1:
	/* 5.3 kbit/s frame */
	return 20;

    default:
	return RTP_NSAMPLES_UNKNOWN;
    }
}

static int 
g723_samples(const unsigned char *buf, int maxlen)
{
    int pos, samples, n;

    for (pos = 0, samples = 0; pos < maxlen; pos += n) {
	samples += 240;
	n = g723_len(buf[pos]);
	if (n == RTP_NSAMPLES_UNKNOWN)
	    return RTP_NSAMPLES_UNKNOWN;
    }
    return samples;
}

static int 
rtp_calc_samples(int codec_id, size_t nbytes, const unsigned char *data)
{

    switch (codec_id) {
	case RTP_PCMU:
	case RTP_PCMA:
	    return nbytes;

	case RTP_G729:
	    return (nbytes / 10) * 80 + (nbytes % 10 == 0 ? 0 : 80);

	case RTP_GSM:
	    return 160 * (nbytes / 33);

	case RTP_G723:
	    return g723_samples(data, nbytes);

	default:
	    return RTP_NSAMPLES_UNKNOWN;
    }
}

static void
rtp_packet_chunk_find_g711(struct rtp_packet *pkt, struct rtp_packet_chunk *ret, int min_nsamples)
{

    ret->nsamples = min_nsamples;
    ret->bytes = min_nsamples;
}

static void
rtp_packet_chunk_find_g729(struct rtp_packet *pkt, struct rtp_packet_chunk *ret, int min_nsamples)
{
    int frames, samples;

    frames = min_nsamples / 80 + ((min_nsamples % 80) == 0 ? 0 : 1);
    samples = frames * 80;

    if (samples >= pkt->nsamples) {
	ret->whole_packet_matched = 1;
	return;
    }
    ret->nsamples = samples;
    ret->bytes = frames * 10;
}

static void
rtp_packet_chunk_find_gsm(struct rtp_packet *pkt, struct rtp_packet_chunk *ret, int min_nsamples)
{
    int frames, samples;

    frames = min_nsamples / 160 + ((min_nsamples % 160) == 0 ? 0 : 1);
    samples = frames * 160;

    if (samples >= pkt->nsamples) {
	ret->whole_packet_matched = 1;
	return;
    }
    ret->nsamples = samples;
    ret->bytes = frames * 33;
}

static void
rtp_packet_chunk_find_g723(struct rtp_packet *pkt, struct rtp_packet_chunk *ret, int min_nsamples)
{
    int frames, samples, pos, found_samples, n;
    unsigned char *buf;

    frames = min_nsamples / 240 + ((min_nsamples % 240) == 0 ? 0 : 1);
    samples = frames * 240;

    pos = 0;
    found_samples = 0;
    if (samples >= pkt->nsamples) {
	ret->whole_packet_matched = 1;
	return;
    }

    buf = &pkt->data.buf[pkt->data_offset];
    while (pos < pkt->data_size && samples > found_samples) {
	found_samples += 240;
	n = g723_len(buf[pos]);
	assert(n != RTP_NSAMPLES_UNKNOWN);
	pos += n;
    }
    ret->nsamples = found_samples;
    ret->bytes = (pos < pkt->data_size ? pos : pkt->data_size);
}

/* 
 * Find the head of the packet with the length at least 
 * of min_nsamples.
 *
 * Warning! When whole packet has been matched the chunk can be uninitialized.
 */
void 
rtp_packet_first_chunk_find(struct rtp_packet *pkt, struct rtp_packet_chunk *ret, int min_nsamples)
{

    assert(pkt->nsamples > min_nsamples);
    ret->whole_packet_matched = 0;

    switch (pkt->data.header.pt) {
    case RTP_PCMU:
    case RTP_PCMA:
	rtp_packet_chunk_find_g711(pkt, ret, min_nsamples);
	break;

    case RTP_G729:
	rtp_packet_chunk_find_g729(pkt, ret, min_nsamples);
	break;

    case RTP_GSM:
	rtp_packet_chunk_find_gsm(pkt, ret, min_nsamples);
	break;

    case RTP_G723:
	rtp_packet_chunk_find_g723(pkt, ret, min_nsamples);
	break;

    default:
	ret->whole_packet_matched = 1;
	break;
    }
}

const char *
rtp_packet_parse_errstr(rtp_parser_err_t ecode)
{
    switch (ecode) {
    case RTP_PARSER_OK:
       return "no error";

    case RTP_PARSER_PTOOSHRT:
       return "packet is too short for RTP header";

    case RTP_PARSER_IHDRVER:
       return "incorrect RTP header version";

    case RTP_PARSER_PTOOSHRTXS:
       return "packet is too short for extended RTP header size";

    case RTP_PARSER_PTOOSHRTXH:
       return "packet is too short for extended RTP header";

    case RTP_PARSER_PTOOSHRTPS:
        return "packet is too short for RTP padding size";

    case RTP_PARSER_PTOOSHRTP:
       return "packet is too short for RTP padding";

    case RTP_PARSER_IPS:
       return "invalid RTP padding size";

    default:
       abort();
    }

    /* NOTREACHED */
    return NULL;
}

rtp_parser_err_t
rtp_packet_parse(struct rtp_packet *pkt)
{
    int padding_size;
    rtp_hdr_ext_t *hdr_ext_ptr;

    padding_size = 0;

    pkt->data_size = 0;
    pkt->data_offset = 0;
    pkt->appendable = 1;
    pkt->nsamples = RTP_NSAMPLES_UNKNOWN;

    if (pkt->size < sizeof(pkt->data.header))
        return RTP_PARSER_PTOOSHRT;

    if (pkt->data.header.version != 2)
        return RTP_PARSER_IHDRVER;

    pkt->data_offset = RTP_HDR_LEN(&pkt->data.header);

    if (pkt->data.header.x != 0) {
        if (pkt->size < pkt->data_offset + sizeof(*hdr_ext_ptr))
            return RTP_PARSER_PTOOSHRTXS;
        hdr_ext_ptr = (rtp_hdr_ext_t *)&pkt->data.buf[pkt->data_offset];
        pkt->data_offset += sizeof(rtp_hdr_ext_t) +
          (ntohs(hdr_ext_ptr->length) * sizeof(hdr_ext_ptr->extension[0]));
    }

    if (pkt->size < pkt->data_offset)
        return RTP_PARSER_PTOOSHRTXH;

    if (pkt->data.header.p != 0) {
        if (pkt->data_offset == pkt->size)
            return RTP_PARSER_PTOOSHRTPS;
        padding_size = pkt->data.buf[pkt->size - 1];
        if (padding_size == 0)
            return RTP_PARSER_IPS;
    }

    if (pkt->size < pkt->data_offset + padding_size)
        return RTP_PARSER_PTOOSHRTP;

    pkt->data_size = pkt->size - pkt->data_offset - padding_size;
    pkt->ts = ntohl(pkt->data.header.ts);
    pkt->seq = ntohs(pkt->data.header.seq);

    if (pkt->data_size == 0)
        return RTP_PARSER_OK;

    pkt->nsamples = rtp_calc_samples(pkt->data.header.pt, pkt->data_size,
      &pkt->data.buf[pkt->data_offset]);
    /* 
     * G.729 comfort noise frame as the last frame causes 
     * packet to be non-appendable
     */
    if (pkt->data.header.pt == RTP_G729 && (pkt->data_size % 10) != 0)
        pkt->appendable = 0;
    return RTP_PARSER_OK;
}

struct rtp_packet *
rtp_packet_alloc()
{
    struct rtp_packet *pkt;

    pkt = rtp_packet_pool;
    if (pkt != NULL)
        rtp_packet_pool = pkt->next;
    else
        pkt = malloc(sizeof(*pkt));
    return pkt;
}

void
rtp_packet_free(struct rtp_packet *pkt)
{

    pkt->next = rtp_packet_pool;
    pkt->prev = NULL;
    rtp_packet_pool = pkt;
}

struct rtp_packet *
rtp_recv(int fd)
{
    struct rtp_packet *pkt;

    pkt = rtp_packet_alloc();

    if (pkt == NULL)
        return NULL;

    pkt->rlen = sizeof(pkt->raddr);
    pkt->size = recvfrom(fd, pkt->data.buf, sizeof(pkt->data.buf), 0, 
      sstosa(&pkt->raddr), &pkt->rlen);

    if (pkt->size == -1) {
	rtp_packet_free(pkt);
	return NULL;
    }

    return pkt;
}

void 
rtp_packet_set_seq(struct rtp_packet *p, uint16_t seq)
{

    p->seq = seq;
    p->data.header.seq = htons(seq);
}

void 
rtp_packet_set_ts(struct rtp_packet *p, uint32_t ts)
{

    p->ts = ts;
    p->data.header.ts = htonl(ts);
}
