/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: cleanup.c,v 1.28 2005/03/20 23:51:52 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>

#include "util.h"

#define __CLEANUP_PRIVATE
#include "cleanup.h"

/* ----------------------------------------------------------------------- *
 * variables, defs ...
 * ----------------------------------------------------------------------- */

#if 0
typedef
struct _state {
    ip4addr_t from, to ;
} STATE ;
#endif /* 0 */

typedef
struct _maint {
    LOOKUP *lookup ;
    time_t     now ;
    int      count ;
} MAINT ;

/* ----------------------------------------------------------------------- *
 * private debugging helpers
 * ----------------------------------------------------------------------- */

static
void show_pool_record (IP4_REC *r, void *p) {
    logger (LOG_INFO, "             pool address = %s (%s).",
            ipaddr_pp (&r->pool_ip),
            r->mac_rec ? ethaddr_pp (&r->mac_rec->real_mac) : "idle");
}

#if 0
static
void show_idle_record (IP4_REC *r, void *p) {

    STATE *s = p ;
    ip4addr_t ip = r ? ntohl (r->pool_ip.s_addr) : 0;

    if (s->from) {
        if (s->to + 1 == ip) {
            s->to = ip ;
            return ;
        }
        if (s->from == s->to) {
            logger (LOG_INFO, "     idle ip pool address = %s.",
                     ip4addr_pp (s->from));
        }
        else {
            logger (LOG_INFO, "   idle ip pool addresses = [%s, %s].",
                     ip4addr_pp (s->from), ip4addr_pp (s->to));
        }
    }

    s->from = s->to = ip ;
}
#endif /* 0 */

/* ----------------------------------------------------------------------- *
 * private maintenance helper
 * ----------------------------------------------------------------------- */

static
void record_maintenance (IP4_REC *r, void *p) {
    MAINT *m = p ;

    if (m->now < r->mac_rec->time2die) return ;

    if (verbosity & CLEANUP_NOISY)
        logger (LOG_INFO, " deleting expired address = %s (%s).",
                ipaddr_pp (&r->pool_ip), ethaddr_pp (&r->mac_rec->real_mac));

    free_pool_address (m->lookup, r);
}

/* ----------------------------------------------------------------------- *
 * Public functions for debugging
 * ----------------------------------------------------------------------- */

void show_pool_records (LOOKUP *l) {
    for_pool_address (l, 1, show_pool_record, 0);
}
  

/* ----------------------------------------------------------------------- *
 * Timeout handler
 * ----------------------------------------------------------------------- */

int maintenance (LOOKUP *l) {
    MAINT m;
    m.now    = time (0);
    m.lookup = l ;
    m.count  = 0 ;
    for_pool_address (m.lookup, 1, record_maintenance, &m);
    return m.count ;
}

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
