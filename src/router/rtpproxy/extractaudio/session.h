/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
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
 * $Id$
 *
 */

#ifndef _SESSION_H_
#define _SESSION_H_

#include <sys/types.h>
#include <sys/socket.h>

#include "rtp.h"
#include "rtpp_record.h"

#define MYQ_INIT(headp) {(headp)->first = (headp)->last = NULL;}
#define MYQ_FIRST(headp) ((headp)->first)
#define MYQ_NEXT(itemp) ((itemp)->next)
#define MYQ_EMPTY(headp) ((headp)->first == NULL)
#define MYQ_FOREACH(itemp, headp) \
  for ((itemp) = (headp)->first; (itemp) != NULL; (itemp) = (itemp)->next)
#define MYQ_FOREACH_REVERSE(itemp, headp) \
  for ((itemp) = (headp)->last; (itemp) != NULL; (itemp) = (itemp)->prev)
#define MYQ_INSERT_HEAD(headp, new_itemp) { \
    (new_itemp)->prev = NULL; \
    (new_itemp)->next = (headp)->first; \
    if ((headp)->first != NULL) { \
      (headp)->first->prev = (new_itemp); \
    } else { \
      (headp)->last = (new_itemp); \
    } \
    (headp)->first = (new_itemp); \
  }
#define MYQ_INSERT_AFTER(headp, itemp, new_itemp) { \
    (new_itemp)->next = (itemp)->next; \
    (new_itemp)->prev = (itemp); \
    (itemp)->next = (new_itemp); \
    if ((new_itemp)->next == NULL) { \
      (headp)->last = (new_itemp); \
    } else { \
      (new_itemp)->next->prev = (new_itemp); \
    } \
  }

struct pkt_hdr_adhoc;

struct packet {
    struct pkt_hdr_adhoc *pkt;
    struct rtp_info parsed;
    rtp_hdr_t *rpkt;
    struct packet *prev;
    struct packet *next;
};
struct session {
    struct packet *first;
    struct packet *last;
};

enum origin {O_CH, A_CH};

struct channel {
    struct session session;
    void *decoder;
    unsigned int skip;
    enum origin origin;
    struct channel *prev;
    struct channel *next;
};
struct channels {
    struct channel *first;
    struct channel *last;
};

#define	RPKT(packet)	((rtp_hdr_t *)((packet)->pkt + 1))
#define RPLOAD(packet)	(((unsigned char *)(packet)->rpkt) + (packet)->parsed.data_offset)
#define RPLEN(packet)	((packet)->parsed.data_size)

struct session *session_lookup(struct channels *, uint32_t);
void channel_insert(struct channels *, struct channel *);

#endif
