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

#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_memdeb_internal.h"
#include "rtpp_memdeb_stats.h"

int
rtpp_memdeb_selftest(void)
{
    void *p;
    int i;
    struct memdeb_stats mds_b, mds_a;

    memset(&mds_b, '\0', sizeof(mds_b));
    memset(&mds_a, '\0', sizeof(mds_a));
    p = malloc(50);
    free(p);
    if (rtpp_memdeb_get_stats(__FILE__, __func__, &mds_b) < 1) {
        RTPP_MEMDEB_REPORT(NULL, "MEMDEB is compiled in but is not working");
        return (-1);
    }
    if (mds_b.nalloc < 1 || mds_b.nfree < 1 || \
      mds_b.balloc < 50 ||  mds_b.bfree < 50) {
        RTPP_MEMDEB_REPORT(NULL, "MEMDEB is compiled in but is not working");
        return (-1);
    }
    for (i = 0; i < 10; i++) {
        p = malloc(16);
        free(p);
    }
    if (rtpp_memdeb_get_stats(__FILE__, __func__, &mds_a) < 2) {
        RTPP_MEMDEB_REPORT(NULL, "MEMDEB is compiled in but is not working");
        return (-1);
    }
    if (RTPP_MD_STATS_CMP(&mds_b, &mds_a) == 0) {
        RTPP_MEMDEB_REPORT(NULL, "MEMDEB is compiled in but is not working");
        return (-1);
    }
    RTPP_MD_STATS_SUB(&mds_a, &mds_b);
    if (mds_a.nalloc != 10 || mds_a.nfree != 10 || \
      mds_a.balloc != 160 ||  mds_a.bfree != 160) {
        RTPP_MEMDEB_REPORT(NULL, "MEMDEB is compiled in but is not working");
        return (-1);
    }
    return (0);
}
