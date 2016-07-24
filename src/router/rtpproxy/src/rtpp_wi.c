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
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rtp.h"
#include "rtpp_network.h"
#include "rtpp_wi.h"
#include "rtpp_wi_private.h"

struct rtpp_wi *
rtpp_wi_malloc(int sock, const void *msg, size_t msg_len, int flags,
  const struct sockaddr *sendto, socklen_t tolen)
{
    struct rtpp_wi *wi;

    wi = malloc(sizeof(struct rtpp_wi) + msg_len + tolen);
    if (wi == NULL) {
        return (NULL);
    }
    wi->free_ptr = wi;
    wi->wi_type = RTPP_WI_TYPE_OPKT;
    wi->sock = sock;
    wi->flags = flags;
    wi->msg = (char *)wi + sizeof(struct rtpp_wi);
    wi->sendto = (struct sockaddr *)((char *)wi->msg + msg_len);
    wi->msg_len = msg_len;
    wi->nsend = 1;
    memcpy(wi->msg, msg, msg_len);
    wi->tolen = tolen;
    memcpy(wi->sendto, sendto, tolen);
    return (wi);
}

struct rtpp_wi *
rtpp_wi_malloc_pkt(int sock, struct rtp_packet *pkt,
  const struct sockaddr *sendto, socklen_t tolen, int nsend)
{
    struct rtpp_wi *wi;

    if (pkt->size + sizeof(struct rtpp_wi) > sizeof(pkt->data.buf)) {
        wi = rtpp_wi_malloc(sock, pkt->data.buf, pkt->size, 0, sendto, tolen);
        rtp_packet_free(pkt);
    } else {
        wi = (struct rtpp_wi *)(pkt->data.buf + pkt->size);
        wi->free_ptr = (struct rtpp_wi *)pkt;
        wi->wi_type = RTPP_WI_TYPE_OPKT;
        wi->sock = sock;
        wi->flags = 0;
        wi->msg = pkt->data.buf;
        wi->msg_len = pkt->size;
        wi->sendto = sstosa(&pkt->raddr);
        wi->tolen = tolen;
        memcpy(wi->sendto, sendto, tolen);
    }
    wi->nsend = nsend;
    return (wi);
}

struct rtpp_wi *
rtpp_wi_malloc_sgnl(int signum, const void *data, size_t datalen)
{
    struct rtpp_wi *wi;

    wi = malloc(sizeof(struct rtpp_wi) + datalen);
    if (wi == NULL) {
        return (NULL);
    }
    memset(wi, '\0', sizeof(struct rtpp_wi));
    wi->free_ptr = wi;
    wi->wi_type = RTPP_WI_TYPE_SGNL;
    wi->flags = signum;
    if (datalen > 0) {
        wi->msg = (char *)wi + datalen;
        wi->msg_len = datalen;
        memcpy(wi->msg, data, datalen);
    }
    return (wi);
}

void *
rtpp_wi_sgnl_get_data(struct rtpp_wi *wi, size_t *datalen)
{

    assert(wi->wi_type == RTPP_WI_TYPE_SGNL);
    if (datalen != NULL) {
        *datalen = wi->msg_len;
    }
    return(wi->msg);
}

void
rtpp_wi_free(struct rtpp_wi *wi)
{

    free(wi->free_ptr);
}
