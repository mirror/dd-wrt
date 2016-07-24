/*
 * Copyright (c) 2013-2014 Sippy Software, Inc., http://www.sippysoft.com
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

#ifdef RTPP_DEBUG
#include <assert.h>
#endif

struct rtpp_list {
    struct rtpp_type_linkable *head;
    struct rtpp_type_linkable *tail;
    unsigned int len;
};

static inline void
rtpp_list_append(struct rtpp_list *lst, void *p)
{
    struct rtpp_type_linkable *elem;

    elem = (struct rtpp_type_linkable *)p;
#ifdef RTPP_DEBUG
#if 0
    assert(RTPP_TYPE_IS_LINKABLE(elem));
#endif
    assert(elem->next == NULL);
#endif
    if (lst->head == NULL) {
        lst->head = lst->tail = elem;
    } else {
        lst->tail->next = elem;
        lst->tail = elem;
    }
    lst->len += 1;
}

#define RTPP_LIST_RESET(lst) {(lst)->head = (lst)->tail = NULL; (lst)->len = 0;}
#define RTPP_LIST_HEAD(lst)  (void *)((lst)->head)
#define RTPP_LIST_IS_EMPTY(slp) ((slp)->len == 0)

#define RTPP_ITER_NEXT(stlp)     ((void *)((stlp)->t.next))
