/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
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
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_command.h"
#include "rtpp_command_private.h"
#include "rtpp_command_parse.h"
#include "rtpp_command_stream.h"
#include "rtpp_util.h"

static void
rtpp_command_stream_compact(struct rtpp_cmd_connection *rcs)
{
    char *cp;
    int clen;

    if (rcs->inbuf_ppos == 0 || rcs->inbuf_epos == 0)
        return;
    if (rcs->inbuf_ppos == rcs->inbuf_epos) {
        rcs->inbuf_ppos = 0;
        rcs->inbuf_epos = 0;
        return;
    }
    cp = &rcs->inbuf[rcs->inbuf_ppos];
    clen = rcs->inbuf_epos - rcs->inbuf_ppos;
    memcpy(rcs->inbuf, cp, clen);
    rcs->inbuf_epos = clen;
    rcs->inbuf_ppos = 0;
}   

int
rtpp_command_stream_doio(struct cfg *cf, struct rtpp_cmd_connection *rcs)
{
    int len, blen;
    char *cp;

    rtpp_command_stream_compact(rcs);
    cp = &(rcs->inbuf[rcs->inbuf_epos]);
    blen = sizeof(rcs->inbuf) - rcs->inbuf_epos;

    for (;;) {
        len = read(rcs->controlfd_in, cp, blen);
        if (len != -1 || (errno != EAGAIN && errno != EINTR))
            break;
    }
    if (len == -1) {
        if (errno != EAGAIN && errno != EINTR)
            rtpp_log_ewrite(RTPP_LOG_ERR, cf->stable->glog, "can't read from control socket");
        return (-1);
    }
    rcs->inbuf_epos += len;
    return (len);
}

struct rtpp_command *
rtpp_command_stream_get(struct cfg *cf, struct rtpp_cmd_connection *rcs,
  int *rval, double dtime, struct rtpp_command_stats *csp)
{
    char **ap;
    char *cp, *cp1;
    int len;
    struct rtpp_command *cmd;

    if (rcs->inbuf_epos == rcs->inbuf_ppos) {
        *rval = EAGAIN;
        return (NULL);
    }
    cp = &(rcs->inbuf[rcs->inbuf_ppos]);
    len = rcs->inbuf_epos - rcs->inbuf_ppos;
    cp1 = memchr(cp, '\n', len);
    if (cp1 == NULL) {
        *rval = EAGAIN;
        return (NULL);
    }

    cmd = malloc(sizeof(struct rtpp_command));
    if (cmd == NULL) {
        *rval = ENOMEM;
        return (NULL);
    }
    memset(cmd, 0, sizeof(struct rtpp_command));

    cmd->controlfd = rcs->controlfd_out;
    cmd->dtime = dtime;
    cmd->csp = csp;
    cmd->umode = 0;

    len = cp1 - cp;
    memcpy(cmd->buf, cp, len);
    cmd->buf[len] = '\0';
    rcs->inbuf_ppos += len + 1;

    rtpp_log_write(RTPP_LOG_DBUG, cf->stable->glog, "received command \"%s\"", cmd->buf);
    csp->ncmds_rcvd.cnt++;

    cp = cmd->buf;
    for (ap = cmd->argv; (*ap = rtpp_strsep(&cp, "\r\n\t ")) != NULL;) {
        if (**ap != '\0') {
            cmd->argc++;
            if (++ap >= &cmd->argv[RTPC_MAX_ARGC])
                break;
        }
    }

    if (cmd->argc < 1) {
        rtpp_log_write(RTPP_LOG_ERR, cf->stable->glog, "command syntax error");
        reply_error(cf, cmd, ECODE_PARSE_1);
        *rval = EINVAL;
        free(cmd);
        return (NULL);
    }

    /* Step I: parse parameters that are common to all ops */
    if (rtpp_command_pre_parse(cf, cmd) != 0) {
        /* Error reply is handled by the rtpp_command_pre_parse() */
        *rval = 0;
        free(cmd);
        return (NULL);
    }

    return (cmd);
}
