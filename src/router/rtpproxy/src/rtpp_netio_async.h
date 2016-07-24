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

#ifndef _RTPP_NETIO_ASYNC_H_
#define _RTPP_NETIO_ASYNC_H_

struct rtpp_anetio_cf;
struct rtp_packet;
struct rtpp_queue;
struct sthread_args;

int rtpp_anetio_sendto(struct rtpp_anetio_cf *, int, const void *, \
  size_t, int, const struct sockaddr *, socklen_t);
int rtpp_anetio_send_pkt(struct sthread_args *, int, \
  const struct sockaddr *, socklen_t, struct rtp_packet *);
void rtpp_anetio_pump(struct rtpp_anetio_cf *);
void rtpp_anetio_pump_q(struct sthread_args *);
struct sthread_args *rtpp_anetio_pick_sender(struct rtpp_anetio_cf *);

struct rtpp_anetio_cf *rtpp_netio_async_init(struct cfg *cf, int);
void rtpp_netio_async_destroy(struct rtpp_anetio_cf *);

#endif
