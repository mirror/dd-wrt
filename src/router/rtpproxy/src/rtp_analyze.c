/*
 * Copyright (c) 2009 Sippy Software, Inc., http://www.sippysoft.com
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

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtp_info.h"
#include "rtp.h"
#include "rtp_analyze.h"
#include "rtpp_math.h"
#include "rtpp_util.h"

static double
rtp_ts2dtime(uint32_t ts)
{

    return ((double)ts) / ((double)8000);
}

void
update_rtpp_stats(struct rtpp_session_stat *stat, rtp_hdr_t *header,
  struct rtp_info *rinfo, double rtime)
{
    uint32_t seq;
    uint16_t idx;
    uint32_t mask;

    seq = rinfo->seq;
    if (stat->last.ssrc != header->ssrc) {
        if (stat->last.pcount > 10) {
            stat->psent += stat->last.max_seq - stat->last.min_seq + 1;
            stat->precvd += stat->last.pcount;
        }
        stat->duplicates += stat->last.duplicates;
        stat->last.duplicates = 0;
        memset(stat->last.seen, '\0', sizeof(stat->last.seen));
        stat->last.ssrc = header->ssrc;
        stat->last.max_seq = stat->last.min_seq =  seq;
        stat->last.base_ts = rinfo->ts;
        stat->last.base_rtime = rtime;
        stat->last.pcount = 1;
        stat->ssrc_changes += 1;
        printf("ssrc_changes=%u, psent=%u, precvd=%u\n", stat->ssrc_changes, stat->psent, stat->precvd);
        return;
    }
    if (ABS(rtime - stat->last.base_rtime - rtp_ts2dtime(rinfo->ts - stat->last.base_ts)) > 0.1) {
        printf("seq %d: delta rtime=%f, delta ts=%f\n", header->seq, rtime - stat->last.base_rtime,
          rtp_ts2dtime(rinfo->ts - stat->last.base_ts));
    }
    seq += stat->last.seq_offset;
    if (stat->last.max_seq % 65536 < 536 && rinfo->seq > 65000) {
        /* Pre-wrap packet received after a wrap */
        seq -= 65536;
    } else if (stat->last.max_seq > 65000 && seq < stat->last.max_seq - 65000) {
        printf("wrap last->max_seq=%u, seq=%u\n", stat->last.max_seq, seq);
        /* Wrap up has happened */
        stat->last.seq_offset += 65536;
        seq += 65536;
        if (stat->last.seq_offset % 131072 == 65536) {
            memset(stat->last.seen + 2048, '\0', sizeof(stat->last.seen) / 2);
        } else {
            memset(stat->last.seen, '\0', sizeof(stat->last.seen) / 2);
        }
    } else if (seq + 536 < stat->last.max_seq || seq > stat->last.max_seq + 536) {
        printf("desync last->max_seq=%u, seq=%u, m=%u\n", stat->last.max_seq, seq, header->m);
        /* Desynchronization has happened. Treat it as a ssrc change */
        if (stat->last.pcount > 10) {
            stat->psent += stat->last.max_seq - stat->last.min_seq + 1;
            stat->precvd += stat->last.pcount;
        }
        stat->duplicates += stat->last.duplicates;
        stat->last.duplicates = 0;
        memset(stat->last.seen, '\0', sizeof(stat->last.seen));
        stat->last.ssrc = header->ssrc;
        stat->last.max_seq = stat->last.min_seq =  seq;
        stat->last.pcount = 1;
        stat->desync_count += 1;
        return;
    }
        /* printf("last->max_seq=%u, seq=%u, m=%u\n", stat->last.max_seq, seq, header->m);*/
    idx = (seq % 131072) >> 5;
    mask = stat->last.seen[idx];
    if (((mask >> (seq & 31)) & 1) != 0) {
        stat->last.duplicates += 1;
        if (stat->last.duplicates > 20)
            {static int b=0; while (b);}
        return;
    }
    stat->last.seen[idx] |= 1 << (rinfo->seq & 31);
    if (seq - stat->last.max_seq != 1)
        printf("delta = %d\n", seq - stat->last.max_seq);
    if (seq >= stat->last.max_seq) {
        stat->last.max_seq = seq;
        stat->last.pcount += 1;
        return;
    }
    if (seq >= stat->last.min_seq) {
        stat->last.pcount += 1;
        return;
    }
    if (stat->last.seq_offset == 0 && seq < stat->last.min_seq) {
        stat->last.min_seq =  seq;
        stat->last.pcount += 1;
        printf("last->min_seq=%u\n", stat->last.min_seq);
        return;
    }
    /* XXX something wrong with the stream */
    abort();
}

void
update_rtpp_totals(struct rtpp_session_stat *stat)
{

    stat->psent += stat->last.max_seq - stat->last.min_seq + 1;
    stat->precvd += stat->last.pcount;
}
