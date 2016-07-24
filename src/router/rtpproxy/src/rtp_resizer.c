/*
 * Copyright (c) 2007 Sippy Software, Inc., http://www.sippysoft.com
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
 */

#include "config.h"

#if defined(HAVE_SYS_ENDIAN_H)
#include <sys/endian.h>
#endif
#include <sys/socket.h>
#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rtp.h"
#include "rtp_info.h"
#include "rtp_resizer.h"
#include "rtpp_proc.h"
#include "rtpp_types.h"
#include "rtpp_stats.h"

struct rtp_resizer {
    int         nsamples_total;

    int         seq_initialized;
    uint16_t    seq;

    int         last_sent_ts_inited;
    uint32_t    last_sent_ts;

    int         tsdelta_inited;
    uint32_t    tsdelta;

    int         output_nsamples;
    int         max_buf_nsamples;

    struct {
        struct rtp_packet *first;
        struct rtp_packet *last;
    } queue;
};

static int
min_nsamples(int codec_id)
{

    switch (codec_id)
    {
    case RTP_GSM:
        return 160; /* 20ms */
    default:
        return 80; /* 10ms */
    }
}

struct rtp_resizer *
rtp_resizer_new(int output_nsamples)
{
    struct rtp_resizer *this;

    this = malloc(sizeof(struct rtp_resizer));
    if (this == NULL)
        return (NULL);
    memset(this, 0, sizeof(struct rtp_resizer));
    rtp_resizer_set_onsamples(this, output_nsamples);
    return (this);
}

void 
rtp_resizer_free(struct rtpp_stats_obj *rtpp_stats, struct rtp_resizer *this)
{
    struct rtp_packet *p;
    struct rtp_packet *p1;
    int nfree;

    nfree = 0;
    p = this->queue.first;
    while (p != NULL) {
        p1 = p;
        p = p->next;
        rtp_packet_free(p1);
        nfree++;
    }
    free(this);
    if (nfree > 0) {
        CALL_METHOD(rtpp_stats, updatebyname, "npkts_resizer_discard", nfree);
    }
}

int
rtp_resizer_get_onsamples(struct rtp_resizer *this)
{

    return(this->output_nsamples);
}

int
rtp_resizer_set_onsamples(struct rtp_resizer *this, int output_nsamples_new)
{
    int output_nsamples_old;

    output_nsamples_old = this->output_nsamples;
    this->output_nsamples = output_nsamples_new;
    this->max_buf_nsamples = output_nsamples_new * 2;
    if (this->max_buf_nsamples < 320) {
        this->max_buf_nsamples = 320;
    }
    return (output_nsamples_old);
}

void
rtp_resizer_enqueue(struct rtp_resizer *this, struct rtp_packet **pkt,
  struct rtpp_proc_rstats *rsp)
{
    struct rtp_packet   *p;
    uint32_t            ref_ts, internal_ts;
    int                 delta;

    p = *pkt;
    if (rtp_packet_parse(p->data.buf, p->size, p->parsed) != RTP_PARSER_OK)
        return;

    if ((*pkt)->parsed->nsamples == RTP_NSAMPLES_UNKNOWN)
        return;

    if (this->last_sent_ts_inited && ts_less((*pkt)->parsed->ts, this->last_sent_ts))
    {
        /* Packet arrived too late. Drop it. */
        rtp_packet_free(*pkt);
        *pkt = NULL;
        rsp->npkts_resizer_discard.cnt++;
        return;
    }
    internal_ts = (*pkt)->rtime * 8000.0;
    if (!this->tsdelta_inited) {
        this->tsdelta = (*pkt)->parsed->ts - internal_ts + 40;
        this->tsdelta_inited = 1;
    }
    else {
        ref_ts = internal_ts + this->tsdelta;
        if (ts_less(ref_ts, (*pkt)->parsed->ts)) {
            this->tsdelta = (*pkt)->parsed->ts - internal_ts + 40;
/*            printf("Sync forward\n"); */
        }
        else if (ts_less((*pkt)->parsed->ts + this->output_nsamples + 160, ref_ts)) 
        {
            delta = (ref_ts - ((*pkt)->parsed->ts + this->output_nsamples + 160)) / 2;
            this->tsdelta -= delta;
/*            printf("Sync backward\n"); */
        }
    }
    if (this->queue.last != NULL) 
    {
        p = this->queue.last; 
        while (p != NULL && ts_less((*pkt)->parsed->ts, p->parsed->ts))
             p = p->prev;

        if (p == NULL) /* head reached */
        {
            (*pkt)->next = this->queue.first;
            (*pkt)->prev = NULL;
            this->queue.first->prev = *pkt;
            this->queue.first = *pkt;
        }
        else if (p == this->queue.last) /* tail of the queue */
        {
            (*pkt)->prev = this->queue.last;
            (*pkt)->next = NULL;
            this->queue.last->next = *pkt;
            this->queue.last = *pkt;
        }
        else { /* middle of the queue */
            (*pkt)->next = p->next;
            (*pkt)->prev = p;
            (*pkt)->next->prev = (*pkt)->prev->next = *pkt;
        }
    }
    else {
        this->queue.first = this->queue.last = *pkt;
        (*pkt)->prev = NULL;
	(*pkt)->next = NULL;
    }
    this->nsamples_total += (*pkt)->parsed->nsamples;
    *pkt = NULL; /* take control over the packet */
}

static void
detach_queue_head(struct rtp_resizer *this)
{

    this->queue.first = this->queue.first->next;
    if (this->queue.first == NULL)
	this->queue.last = NULL;
    else
	this->queue.first->prev = NULL;
}

static void
append_packet(struct rtp_packet *dst, struct rtp_packet *src)
{

    memcpy(&dst->data.buf[dst->parsed->data_offset + dst->parsed->data_size], 
      &src->data.buf[src->parsed->data_offset], src->parsed->data_size);
    dst->parsed->nsamples += src->parsed->nsamples;
    dst->parsed->data_size += src->parsed->data_size;
    dst->size += src->parsed->data_size;
    dst->parsed->appendable = src->parsed->appendable;
}

static void 
append_chunk(struct rtp_packet *dst, struct rtp_packet *src, const struct rtp_packet_chunk *chunk)
{

    /* Copy chunk */
    memcpy(&dst->data.buf[dst->parsed->data_offset + dst->parsed->data_size], 
      &src->data.buf[src->parsed->data_offset], chunk->bytes);
    dst->parsed->nsamples += chunk->nsamples;
    dst->parsed->data_size += chunk->bytes;
    dst->size += chunk->bytes;

    /* Truncate the source packet */
    src->parsed->nsamples -= chunk->nsamples;
    rtp_packet_set_ts(src, src->parsed->ts + chunk->nsamples);
    src->parsed->data_size -= chunk->bytes;
    src->size -= chunk->bytes;
    memmove(&src->data.buf[src->parsed->data_offset],
      &src->data.buf[src->parsed->data_offset + chunk->bytes], src->parsed->data_size);
}

static void 
move_chunk(struct rtp_packet *dst, struct rtp_packet *src, const struct rtp_packet_chunk *chunk)
{
    /* Copy chunk */
    memcpy(&dst->data.buf[dst->parsed->data_offset],
      &src->data.buf[src->parsed->data_offset], chunk->bytes);
    dst->parsed->nsamples = chunk->nsamples;
    dst->parsed->data_size = chunk->bytes;
    dst->size = dst->parsed->data_size + dst->parsed->data_offset;

    /* Truncate the source packet */
    src->parsed->nsamples -= chunk->nsamples;
    rtp_packet_set_ts(src, src->parsed->ts + chunk->nsamples);
    src->parsed->data_size -= chunk->bytes;
    src->size -= chunk->bytes;
    memmove(&src->data.buf[src->parsed->data_offset],
      &src->data.buf[src->parsed->data_offset + chunk->bytes], src->parsed->data_size);
}

struct rtp_packet *
rtp_resizer_get(struct rtp_resizer *this, double dtime)
{
    struct rtp_packet *ret = NULL;
    struct rtp_packet *p;
    uint32_t    ref_ts;
    int         count = 0;
    int         split = 0;
    int         nsamples_left;
    int         output_nsamples;
    int         min;
    struct      rtp_packet_chunk chunk;

    if (this->queue.first == NULL)
        return NULL;

    ref_ts = (dtime * 8000.0) + this->tsdelta;

    /* Wait untill enough data has arrived or timeout occured */
    if (this->nsamples_total < this->output_nsamples &&
        ts_less(ref_ts, this->queue.first->parsed->ts + this->max_buf_nsamples))
    {
        return NULL;
    }

    output_nsamples = this->output_nsamples;
    min = min_nsamples(this->queue.first->data.header.pt);
    if (output_nsamples < min) {
        output_nsamples = min;
    } else if (output_nsamples % min != 0) {
        output_nsamples += (min - (output_nsamples % min));
    }

    /* Aggregate the output packet */
    while ((ret == NULL || ret->parsed->nsamples < output_nsamples) && this->queue.first != NULL)
    {
        p = this->queue.first;
        if (ret == NULL) 
        {
            /* Look if the first packet is to be split */
            if (p->parsed->nsamples > output_nsamples) {
		rtp_packet_first_chunk_find(p, &chunk, output_nsamples);
		if (chunk.whole_packet_matched) {
		    ret = p;
		    detach_queue_head(this);
		} else {
		    ret = rtp_packet_alloc();
		    if (ret == NULL)
			break;
		    memcpy(ret, p, offsetof(struct rtp_packet, data.buf) + sizeof(rtp_hdr_t));
		    move_chunk(ret, p, &chunk);
		    ++split;
		}
		if (!this->seq_initialized) {
		    this->seq = ret->parsed->seq;
		    this->seq_initialized = 1;
		}
		++count;
		break;
	    }
        }
        else /* ret != NULL */
        {
            /* detect holes and payload changes in RTP stream */
            if ((ret->parsed->ts + ret->parsed->nsamples) != p->parsed->ts ||
                ret->data.header.pt != p->data.header.pt)
            {
                break;
            }
            nsamples_left = output_nsamples - ret->parsed->nsamples;

            /* Break the input packet into pieces to create output packet 
             * of specified size */
            if (nsamples_left > 0 && nsamples_left < p->parsed->nsamples) {
		rtp_packet_first_chunk_find(p, &chunk, nsamples_left);
		if (chunk.whole_packet_matched) {
		    /* Prevent RTP packet buffer overflow */
		    if ((ret->size + p->parsed->data_size) > sizeof(ret->data.buf))
			break;
		    append_packet(ret, p);
		    detach_queue_head(this);
		    rtp_packet_free(p);
		}
		else {
		    /* Prevent RTP packet buffer overflow */
		    if ((ret->size + chunk.bytes) > sizeof(ret->data.buf))
			break;
		    /* Append chunk to output */
		    append_chunk(ret, p, &chunk);
		    ++split;
		}
		++count;
		break;
            }
        }
        ++count;

        /*
         * Prevent RTP packet buffer overflow 
         */
        if (ret != NULL && (ret->size + p->parsed->data_size) > sizeof(ret->data.buf))
            break;

        /* Detach head packet from the queue */
	detach_queue_head(this);

        /*
         * Add the packet to the output
         */
        if (ret == NULL) {
            ret = p; /* use the first packet as the result container */
            if (!this->seq_initialized) {
                this->seq = p->parsed->seq;
                this->seq_initialized = 1;
            }
        }
        else {
	    append_packet(ret, p);
            rtp_packet_free(p);
        }
	/* Send non-appendable packet immediately */
	if (!ret->parsed->appendable)
	    break;
    }
    if (ret != NULL) {
	this->nsamples_total -= ret->parsed->nsamples;
	rtp_packet_set_seq(ret, this->seq);
	++this->seq;
	this->last_sent_ts_inited = 1;
	this->last_sent_ts = ret->parsed->ts + ret->parsed->nsamples;
/*
	printf("Payload %d, %d packets aggregated, %d splits done, final size %dms\n", ret->data.header.pt, count, split, ret->parsed->nsamples / 8);
*/
    }
    return ret;
}
