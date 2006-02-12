/*
 * Copyright (c) 2003, 2004 Maxim Sobolev
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
 * $Id: rtpp_record.c,v 1.1 2005/03/06 19:03:52 sobomax Exp $
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_log.h"
#include "rtpp_record.h"
#include "rtpp_session.h"
#include "rtpp_util.h"

struct rtpp_record_channel {
    char spath[PATH_MAX + 1];
    char rpath[PATH_MAX + 1];
    int fd;
    int needspool;
};

#define	RRC_CAST(x)	((struct rtpp_record_channel *)(x))

void *
ropen(struct rtpp_session *sp, const char *rdir, const char *sdir, int orig)
{
    struct rtpp_record_channel *rrc;

    rrc = malloc(sizeof(*rrc));
    if (rrc == NULL) {
        rtpp_log_ewrite(RTPP_LOG_ERR, sp->log, "can't allocate memory");
        return NULL;
    }
    memset(rrc, 0, sizeof(*rrc));

    if (sdir == NULL) {
        sdir = rdir;
        rrc->needspool = 0;
    } else {
        rrc->needspool = 1;
        sprintf(rrc->rpath, "%s/%s=%s.%c.%s", rdir, sp->call_id, sp->tag,
          (orig != 0) ? 'o' : 'a', (sp->rtcp != NULL) ? "rtp" : "rtcp");
    }
    sprintf(rrc->spath, "%s/%s=%s.%c.%s", sdir, sp->call_id, sp->tag,
      (orig != 0) ? 'o' : 'a', (sp->rtcp != NULL) ? "rtp" : "rtcp");
    rrc->fd = open(rrc->spath, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    if (rrc->fd == -1) {
	rtpp_log_ewrite(RTPP_LOG_ERR, sp->log, "can't open file %s for writing",
	  rrc->spath);
	free(rrc);
	return NULL;
    }
    return (void *)(rrc);
}

void
rwrite(struct rtpp_session *sp, void *rrc, struct sockaddr *saddr, void *buf, int len)
{
    struct iovec v[2];
    struct pkt_hdr hdr;
    int rval;

    if (RRC_CAST(rrc)->fd == -1)
        return;

    memset(&hdr, 0, sizeof(hdr));

    hdr.time = getctime();
    if (hdr.time == -1) {
	rtpp_log_ewrite(RTPP_LOG_ERR, sp->log, "can't get current time");
	goto fatal_error;
    }

    switch (saddr->sa_family) {
    case AF_INET:
	hdr.addr.in4.sin_family = saddr->sa_family;
	hdr.addr.in4.sin_port = satosin(saddr)->sin_port;
	hdr.addr.in4.sin_addr = satosin(saddr)->sin_addr;
	break;

    case AF_INET6:
	hdr.addr.in6.sin_family = saddr->sa_family;
	hdr.addr.in6.sin_port = satosin6(saddr)->sin6_port;
	hdr.addr.in6.sin_addr = satosin6(saddr)->sin6_addr;
	break;

    default:
	abort();
    }

    hdr.plen = len;

    v[0].iov_base = (void *)&hdr;
    v[0].iov_len = sizeof(hdr);
    v[1].iov_base = buf;
    v[1].iov_len = len;

    rval = writev(RRC_CAST(rrc)->fd, v, 2);
    if (rval != -1)
	return;

    rtpp_log_ewrite(RTPP_LOG_ERR, sp->log, "error while recording session (%s)",
      (sp->rtcp != NULL) ? "RTP" : "RTCP");
fatal_error:
    /* Prevent futher writing if error happens */
    close(RRC_CAST(rrc)->fd);
    RRC_CAST(rrc)->fd = -1;
}

void
rclose(struct rtpp_session *sp, void *rrc)
{

    if (RRC_CAST(rrc)->fd != -1)
        close(RRC_CAST(rrc)->fd);
    if (RRC_CAST(rrc)->needspool == 1)
        if (rename(RRC_CAST(rrc)->spath, RRC_CAST(rrc)->rpath) == -1)
            rtpp_log_ewrite(RTPP_LOG_ERR, sp->log, "can't move "
              "session record from spool into permanent storage");
    free(rrc);
}
