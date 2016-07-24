/*
 * Copyright (c) 2015 Sippy Software, Inc., http://www.sippysoft.com
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

struct memdeb_stats {
    int64_t nalloc;
    int64_t balloc;
    int64_t nunalloc_baseln;
    int64_t bunalloc_baseln;
    int64_t nfree;
    int64_t bfree;
    int64_t nrealloc;
    int64_t brealloc;
    int64_t afails;
};

/* a += b */
#define RTPP_MD_STATS_ADD(a, b) { \
    (a)->nalloc += (b)->nalloc; \
    (a)->balloc += (b)->balloc; \
    (a)->nunalloc_baseln += (b)->nunalloc_baseln; \
    (a)->bunalloc_baseln += (b)->bunalloc_baseln; \
    (a)->nfree += (b)->nfree; \
    (a)->bfree += (b)->bfree; \
    (a)->nrealloc += (b)->nrealloc; \
    (a)->brealloc += (b)->brealloc; \
    (a)->afails += (b)->afails; \
}

/* a -= b */
#define RTPP_MD_STATS_SUB(a, b) { \
    (a)->nalloc -= (b)->nalloc; \
    (a)->balloc -= (b)->balloc; \
    (a)->nunalloc_baseln -= (b)->nunalloc_baseln; \
    (a)->bunalloc_baseln -= (b)->bunalloc_baseln; \
    (a)->nfree -= (b)->nfree; \
    (a)->bfree -= (b)->bfree; \
    (a)->nrealloc -= (b)->nrealloc; \
    (a)->brealloc -= (b)->brealloc; \
    (a)->afails -= (b)->afails; \
}

/* (a == b) ? 0: 1 */
#define RTPP_MD_STATS_CMP(a, b) ( \
    (((a)->nalloc == (b)->nalloc) && \
     ((a)->balloc == (b)->balloc) && \
     ((a)->nunalloc_baseln == (b)->nunalloc_baseln) && \
     ((a)->bunalloc_baseln == (b)->bunalloc_baseln) && \
     ((a)->nfree == (b)->nfree) && \
     ((a)->bfree == (b)->bfree) && \
     ((a)->nrealloc == (b)->nrealloc) && \
     ((a)->brealloc == (b)->brealloc) && \
     ((a)->afails == (b)->afails)) ? 0 : 1)

int rtpp_memdeb_get_stats(const char *, const char *, struct memdeb_stats *);
