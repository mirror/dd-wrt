/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
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
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_util.h"

int
handle_copy(struct cfg *cf, struct rtpp_session *spa, int idx, char *rname,
  int record_single_file)
{
    int remote;

    remote = (rname != NULL && strncmp("udp:", rname, 4) == 0)? 1 : 0;

    if (remote == 0 && (record_single_file != 0 || spa->record_single_file != 0)) {
        if (spa->rrcs[idx] != NULL)
            return (-1);
        spa->record_single_file = 1;
        if (spa->rrcs[NOT(idx)] != NULL) {
            spa->rrcs[idx] = spa->rrcs[NOT(idx)];
        } else{
            spa->rrcs[idx] = ropen(cf, spa, rname, idx);
            if (spa->rrcs[idx] == NULL) {
                return (-1);
            }
            rtpp_log_write(RTPP_LOG_INFO, spa->log,
              "starting recording RTP session on port %d", spa->ports[idx]);
        }
        assert(spa->rtcp->rrcs[idx] == NULL);
        if (cf->stable->rrtcp != 0) {
            spa->rtcp->rrcs[idx] = spa->rrcs[idx];
            rtpp_log_write(RTPP_LOG_INFO, spa->log,
              "starting recording RTCP session on port %d", spa->rtcp->ports[idx]);
        }
        return (0);
    }

    if (spa->rrcs[idx] == NULL) {
        spa->rrcs[idx] = ropen(cf, spa, rname, idx);
        if (spa->rrcs[idx] == NULL) {
            return (-1);
        }
        rtpp_log_write(RTPP_LOG_INFO, spa->log,
          "starting recording RTP session on port %d", spa->ports[idx]);
    }
    if (spa->rtcp->rrcs[idx] == NULL && cf->stable->rrtcp != 0) {
        spa->rtcp->rrcs[idx] = ropen(cf, spa->rtcp, rname, idx);
        if (spa->rtcp->rrcs[idx] == NULL) {
            return (-1);
        }
        rtpp_log_write(RTPP_LOG_INFO, spa->log,
          "starting recording RTCP session on port %d", spa->rtcp->ports[idx]);
    }
    return (0);
}
