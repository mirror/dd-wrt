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
 * $Id: rtp_resizer.h,v 1.1 2007/11/16 08:43:26 sobomax Exp $
 *
 */

#ifndef __RTP_RESIZER_H
#define __RTP_RESIZER_H

struct rtp_resizer {
    int         nsamples_total;

    int         seq_initialized;
    uint16_t    seq;

    int         last_sent_ts_inited;
    uint32_t    last_sent_ts;

    int         tsdelta_inited;
    uint32_t    tsdelta;

    int         output_nsamples;

    struct {
	struct rtp_packet *first;
	struct rtp_packet *last;
    } queue;
};

void rtp_resizer_enqueue(struct rtp_resizer *, struct rtp_packet **);
struct rtp_packet *rtp_resizer_get(struct rtp_resizer *, double);

void rtp_resizer_free(struct rtp_resizer *);

#define is_rtp_resizer_enabled(resizer) ((resizer).output_nsamples > 0)

#endif /* __RTP_RESIZER_H */
