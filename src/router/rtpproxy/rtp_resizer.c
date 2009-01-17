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
 * $Id: rtp_resizer.c,v 1.6 2009/01/12 11:36:40 sobomax Exp $
 *
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdint.h>

#include "rtp.h"
#include "rtp_resizer.h"

static int
max_nsamples(int codec_id)
{

    switch (codec_id)
    {
    case RTP_GSM:
        return 160; /* 20ms */
    default:
        return 0; /* infinite */
    }
}

void 
rtp_resizer_free(struct rtp_resizer *this)
{
    struct rtp_packet *p;
    struct rtp_packet *p1;

    p = this->queue.first;
    while (p != NULL) {
        p1 = p;
        p = p->next;
        rtp_packet_free(p1);
    }
}

void
rtp_resizer_enqueue(struct rtp_resizer *this, struct rtp_packet **pkt)
{
    struct rtp_packet   *p;
    uint32_t            ref_ts, internal_ts;
    int                 delta;

    if (rtp_packet_parse(*pkt) != RTP_PARSER_OK)
        return;

    if ((*pkt)->nsamples == RTP_NSAMPLES_UNKNOWN)
        return;

    if (this->last_sent_ts_inited && ts_less((*pkt)->ts, this->last_sent_ts))
    {
        /* Packet arrived too late. Drop it. */
        rtp_packet_free(*pkt);
        *pkt = NULL;
        return;
    }
    internal_ts = (*pkt)->rtime * 8000.0;
    if (!this->tsdelta_inited) {
        this->tsdelta = (*pkt)->ts - internal_ts + 40;
        this->tsdelta_inited = 1;
    }
    else {
        ref_ts = internal_ts + this->tsdelta;
        if (ts_less(ref_ts, (*pkt)->ts)) {
            this->tsdelta = (*pkt)->ts - internal_ts + 40;
/*            printf("Sync forward\n"); */
        }
        else if (ts_less((*pkt)->ts + this->output_nsamples + 160, ref_ts)) 
        {
            delta = (ref_ts - ((*pkt)->ts + this->output_nsamples + 160)) / 2;
            this->tsdelta -= delta;
/*            printf("Sync backward\n"); */
        }
    }
    if (this->queue.last != NULL) 
    {
        p = this->queue.last; 
        while (p != NULL && ts_less((*pkt)->ts, p->ts))
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
    this->nsamples_total += (*pkt)->nsamples;
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

    memcpy(&dst->data.buf[dst->data_offset + dst->data_size], 
      &src->data.buf[src->data_offset], src->data_size);
    dst->nsamples += src->nsamples;
    dst->data_size += src->data_size;
    dst->size += src->data_size;
    dst->appendable = src->appendable;
}

static void 
append_chunk(struct rtp_packet *dst, struct rtp_packet *src, const struct rtp_packet_chunk *chunk)
{

    /* Copy chunk */
    memcpy(&dst->data.buf[dst->data_offset + dst->data_size], 
      &src->data.buf[src->data_offset], chunk->bytes);
    dst->nsamples += chunk->nsamples;
    dst->data_size += chunk->bytes;
    dst->size += chunk->bytes;

    /* Truncate the source packet */
    src->nsamples -= chunk->nsamples;
    rtp_packet_set_ts(src, src->ts + chunk->nsamples);
    src->data_size -= chunk->bytes;
    src->size -= chunk->bytes;
    memmove(&src->data.buf[src->data_offset],
      &src->data.buf[src->data_offset + chunk->bytes], src->data_size);
}

static void 
move_chunk(struct rtp_packet *dst, struct rtp_packet *src, const struct rtp_packet_chunk *chunk)
{
    /* Copy chunk */
    memcpy(&dst->data.buf[dst->data_offset],
      &src->data.buf[src->data_offset], chunk->bytes);
    dst->nsamples = chunk->nsamples;
    dst->data_size = chunk->bytes;
    dst->size = dst->data_size + dst->data_offset;

    /* Truncate the source packet */
    src->nsamples -= chunk->nsamples;
    rtp_packet_set_ts(src, src->ts + chunk->nsamples);
    src->data_size -= chunk->bytes;
    src->size -= chunk->bytes;
    memmove(&src->data.buf[src->data_offset],
      &src->data.buf[src->data_offset + chunk->bytes], src->data_size);
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
    int         max;
    struct      rtp_packet_chunk chunk;

    if (this->queue.first == NULL)
        return NULL;

    ref_ts = (dtime * 8000.0) + this->tsdelta;

    /* Wait untill enough data has arrived or timeout occured */
    if (this->nsamples_total < this->output_nsamples &&
        ts_less(ref_ts, this->queue.first->ts + this->output_nsamples + 160))
    {
        return NULL;
    }

    output_nsamples = this->output_nsamples;
    max = max_nsamples(this->queue.first->data.header.pt);
    if (max > 0 && output_nsamples > max)
        output_nsamples = max;

    /* Aggregate the output packet */
    while ((ret == NULL || ret->nsamples < output_nsamples) && this->queue.first != NULL)
    {
        p = this->queue.first;
        if (ret == NULL) 
        {
            /* Look if the first packet is to be split */
            if (p->nsamples > output_nsamples) {
		rtp_packet_first_chunk_find(p, &chunk, output_nsamples);
		if (chunk.whole_packet_matched) {
		    ret = p;
		    detach_queue_head(this);
		} else {
		    ret = rtp_packet_alloc();
		    if (ret == NULL)
			break;
		    memcpy(ret, p, offsetof(struct rtp_packet, data.buf));
		    move_chunk(ret, p, &chunk);
		    ++split;
		}
		if (!this->seq_initialized) {
		    this->seq = ret->seq;
		    this->seq_initialized = 1;
		}
		++count;
		break;
	    }
        }
        else /* ret != NULL */
        {
            /* detect holes and payload changes in RTP stream */
            if ((ret->ts + ret->nsamples) != p->ts ||
                ret->data.header.pt != p->data.header.pt)
            {
                break;
            }
            nsamples_left = output_nsamples - ret->nsamples;

            /* Break the input packet into pieces to create output packet 
             * of specified size */
            if (nsamples_left > 0 && nsamples_left < p->nsamples) {
		rtp_packet_first_chunk_find(p, &chunk, nsamples_left);
		if (chunk.whole_packet_matched) {
		    /* Prevent RTP packet buffer overflow */
		    if ((ret->size + p->data_size) > sizeof(ret->data.buf))
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
        if (ret != NULL && (ret->size + p->data_size) > sizeof(ret->data.buf))
            break;

        /* Detach head packet from the queue */
	detach_queue_head(this);

        /*
         * Add the packet to the output
         */
        if (ret == NULL) {
            ret = p; /* use the first packet as the result container */
            if (!this->seq_initialized) {
                this->seq = p->seq;
                this->seq_initialized = 1;
            }
        }
        else {
	    append_packet(ret, p);
            rtp_packet_free(p);
        }
	/* Send non-appendable packet immediately */
	if (!ret->appendable)
	    break;
    }
    if (ret != NULL) {
	this->nsamples_total -= ret->nsamples;
	rtp_packet_set_seq(ret, this->seq);
	++this->seq;
	this->last_sent_ts_inited = 1;
	this->last_sent_ts = ret->ts + ret->nsamples;
/*
	printf("Payload %d, %d packets aggregated, %d splits done, final size %dms\n", ret->data.header.pt, count, split, ret->nsamples / 8);
*/
    }
    return ret;
}
