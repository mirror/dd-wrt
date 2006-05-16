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
 * $Id: lookup.c,v 1.22 2005/04/30 12:00:21 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util.h"
#include "pool.h"
#include "device.h"

#define __LOOKUP_PRIVATE
#include "lookup.h"


/* ----------------------------------------------------------------------- *
 * local helpers
 * ----------------------------------------------------------------------- */

static
IP4_HOOK *new_pool_address (LOOKUP *l) {

    IP4_HOOK *p ;
    MAC_HOOK *m ;

    if (l->pool_free <= 0)
        return 0;

    if (l->pool_cache) {
        p = l->pool_cache ;
        l->pool_cache = 0 ;
        goto found_ok ;
    }
    
    /* serach for a free record */

    {
        IP4_HOOK *q = 0;
        p = l->pool_recs + l->pool_free;
        while (-- p >= l->pool_recs) {
            if (p->busy) continue ;
            /* try to find a yet unused record */
            if (p->ip4_rec.mac_rec == 0) goto found_ok;
            /* otherwise remember this one and use it, below */
            if (q == 0) p = q ;
        }
    
        p = l->pool_recs + l->pool_free;
        do {
            if (p->busy) continue ;
            /* try to find a yet unused record */
            if (p->ip4_rec.mac_rec == 0) goto found_ok;
            /* otherwise remember this one and use it, below */
            if (q == 0) p = q ;
        }
        while (++ p < l->pool_recs + l->pool_size);

        if ((p = q) == 0)
            return 0;
    }

found_ok:

    /* there might be some garbage to recycle */
    if ((m = (MAC_HOOK*)p->ip4_rec.mac_rec) != 0) {
        MAC_HOOK *verify  = mac_delete (l->maclist, &m->mac_rec.real_mac);
        assert (m == verify);
        p->ip4_rec.mac_rec = 0;
        free (m);
    }

    /* we return that record but do not enable it, yet */
    return p ;
}


/* notify that we have seen that record, recently */
static
void visited_mac_address (LOOKUP        *l,
                          MAC_REC *mac_rec) {
    mac_rec->time2die = time (0) + l->mac_rec_lease ;
}


/* ----------------------------------------------------------------------- *
 * Lookup functions
 * ----------------------------------------------------------------------- */

/* loop over all active pool adresses */
void for_pool_address (LOOKUP                    *l,
                       int               busy_state,
                       void (*cb) (IP4_REC*, void*),
                       void                      *d) {

    if (l && cb) {
        int    busy = busy_state ? 1 : 0 ;
        int       n = l->pool_size ;
        int records = busy ? (n - l->pool_free) : l->pool_free ;
        
        if (records == 0)
            return ;
        
        if (busy && records < n/3) {
            /* search by mac index */
            MAC_WALK  *w = mac_walk_init (l->maclist, 0);
            MAC_HOOK **M = (MAC_HOOK**)mac_this (w) ;
            do 
                if ((*M)->busy)
                    (*cb) ((*M)->mac_rec.ip4_rec, d) ;
            while ((M = (MAC_HOOK**) mac_next (w)) != 0);
            mac_walk_close (w);
        }
        else {
            /* linearly step through the ip pool records */
            IP4_HOOK *p = l->pool_recs ;
            do
                if (p->busy == busy)
                    (*cb) (&p->ip4_rec, d) ;
                else
                    -- records ;
            while (++ p, records && -- n > 0);
        }
    }
}


/*
 * The cleanup function is save when called by an interrup handler.
 * It does not change any pool access index trees but flags the
 * record idle, only.
 */
   
void free_pool_address (LOOKUP    *l,
                        IP4_REC *rec) {

    IP4_HOOK *ip4_hook = (IP4_HOOK*) rec ;

    if (ip4_hook->busy) {
        MAC_HOOK *mac_hook = (MAC_HOOK*)ip4_hook->ip4_rec.mac_rec ;
        ip4_hook->busy = 0 ;
        mac_hook->busy = 0 ;
    }
}


/*
 * Step through the address pool and recycle mac garbage records.
 * This function not safe when called by an interrupt handler.
 */

unsigned purge_pool_garbage (LOOKUP *l) {
    int count = 0;

    if (l) {
        /* linearly step through the ip pool records */
        MAC_HOOK *m ;
        IP4_HOOK *p = l->pool_recs ;
        int       n = l->pool_size ;

        do  /* check for unrecycled garbage */
            if (p->busy != 0 && (m = (MAC_HOOK*) p->ip4_rec.mac_rec) != 0) {
                MAC_HOOK *v = mac_delete (l->maclist, &m->mac_rec.real_mac);
                assert (m == v);
                p->ip4_rec.mac_rec = 0;
                free (m);
                count ++ ;
            }
        while (++ p, -- n > 0);
    }

    return count ;
}


IP4_REC *find_pool_address (LOOKUP          *l,
                            struct in_addr *ip) {

    IP4_HOOK **P = (IP4_HOOK**)ip4_lookup (l->iplist, ip);

    /* make sure that the record is activated */
    if (P == 0 || (*P)->busy == 0) return 0;
    
    /* remember that we have seen that record, recently */
    visited_mac_address (l, (*P)->ip4_rec.mac_rec);

    return &(*P)->ip4_rec ;
}


int num_slots      (LOOKUP *l) { return l->pool_size ; }
int num_slots_used (LOOKUP *l) { return l->pool_size - l->pool_free ; }


int max_slot_used (LOOKUP *l) {
    int slot ;

    if (l->pool_free < l->pool_size)
        for (slot = l->pool_size - 1; slot >= 0; slot --)
            if (l->pool_recs [slot].busy) return slot ;

    return -1;
}

int min_slot_used (LOOKUP *l) {
    int slot ;

    if (l->pool_free < l->pool_size)
        for (slot = 0; slot < l->pool_size; slot ++)
            if (l->pool_recs [slot].busy) return slot ;

    return -1;
}


IP4_REC *use_pool_slot (LOOKUP *l,
                         int slot) {
    if (0 <= slot && slot < l->pool_size)
        return (IP4_REC *) (l->pool_recs + slot) ;
    return 0;
}
        


IP4_REC *find_pool_slot (LOOKUP *l,
                         int slot) {
    if (0 <= slot && slot < l->pool_size) {
        IP4_HOOK * ip4 = l->pool_recs + slot ;
        if (ip4->busy)
            return (IP4_REC *) ip4;
    }
    return 0 ;
}


int get_slot_number (LOOKUP    *l,
                     IP4_REC *ip4) {
    return ip4 ? (IP4_HOOK*)ip4 - l->pool_recs : -1 ;
}


MAC_REC *find_mac_address (LOOKUP              *l,
                           struct ether_addr *mac) {

    MAC_HOOK **M = (MAC_HOOK**)mac_lookup (l->maclist, mac);

    /* make sure that this is no garbage record not recycled, yet */
    if (M == 0 || (*M)->busy == 0) return 0 ;

    /* remember that we have seen that record, recently */
    visited_mac_address (l, &(*M)->mac_rec);
    
    return &(*M)->mac_rec;
}



MAC_REC *make_mac_address (LOOKUP              *l,
                           struct in_addr     *ip,
                           struct ether_addr *mac) {
    MAC_HOOK **M ;
    IP4_HOOK *ip4_hook ;

    if (l->pool_free <= 0)
        /* no more addresses in the pool, so we can only lookup */
        return find_mac_address (l, mac);

    /* create mac lookup entry when necessary */
    M = (MAC_HOOK**) mac_fetch (l->maclist, mac) ;

    if (*M == 0 || (*M)->busy ==0) {

        /* get clean MAC record, initialise M and ip4_hook */
        if (*M == 0) {
            /* brand new new entry */
            *M       = XMALLOC (MAC_HOOK);
            ip4_hook = new_pool_address (l) ;
        }
        else {
            /* may know this record, already */
            if ((ip4_hook = (IP4_HOOK*)(*M)->mac_rec.ip4_rec) != 0) {
                /* reuse the old record if possible */
                if (ip4_hook->busy)
                    ip4_hook = new_pool_address (l);
                else
                    ip4_hook->busy = 1 ;
            }
            XZERO (**M) ;
        }
        
        /* populate mac record and link it as leaf */
        memcpy         (& (*M)->mac_rec.real_mac, mac, sizeof (*mac));
        if (ip) memcpy (& (*M)->mac_rec.real_ip,   ip, sizeof  (*ip));

        /* associate with pool address */
        (*M)    ->mac_rec.ip4_rec = (IP4_REC*) ip4_hook ;
        ip4_hook->ip4_rec.mac_rec = (MAC_REC*) *M ;

        /* enable it */
        l->pool_free -- ;
        ip4_hook->busy = 1;
        (*M)    ->busy = 1;
    }
    
    /* remember that we have seen that record, recently */
    visited_mac_address (l, &(*M)->mac_rec);

    return &(*M)->mac_rec;
}

/* ----------------------------------------------------------------------- *
 * Lookup Constructor/Destructor
 * ----------------------------------------------------------------------- */

LOOKUP *lookup_init (IP4_POOL  *pool,
                     IN_DEV *arp_dev) {
    LOOKUP   *l ;
    IP4_WALK *w ;

    if (pool == 0)
        return 0;

    l                    = XMALLOC (LOOKUP);
    l->maclist           = mac_pool_init ();
    l->iplist            = pool ;
    l->mac_rec_lease     = DEFAULT_REC_LEASE ;
    l->dev               = arp_dev;
    
    /* initialise the address pool */
    if ((w = ip4_walk_init (pool, 0)) != 0) {

        /* Count the number of records, needed and allocate memory. */
        do l->pool_size ++; while (ip4_next (w)) ;
        /* We are wraped around automatically */

        /* allocate memory space for the records as needed */
        l = (LOOKUP*) xrealloc
            (l, sizeof (LOOKUP) + (l->pool_size - 1) * sizeof (IP4_HOOK));
        memset (l->pool_recs, 0, l->pool_size * sizeof (IP4_HOOK));

        /*
         * It is not possible to assign records within one pass as the
         * memory location may change when reallocating memory space.
         */
        {
            IP4_HOOK **P = (IP4_HOOK**) ip4_this (w) ;
            do {
                *P = l->pool_recs + (l->pool_free ++);
                (*P)->ip4_rec.pool_ip.s_addr = htonl (ip4_inx2ip4addr (w)) ;
            }
            while ((P = (IP4_HOOK**) ip4_next (w)) != 0);
        }
        ip4_walk_close (w);
    }
    
    return l;
}


IP4_POOL *lookup_close (LOOKUP *l) {
    IP4_POOL  *p ;
    IP4_WALK  *w ;
    IP4_HOOK **P ;

    if (l == 0)
        return 0 ;

    mac_pool_close (l->maclist, free);
    
    /* we need to reset all the leaf record pointers */
    w = ip4_walk_init (l->iplist, 0) ;
    P = (IP4_HOOK**) ip4_this (w) ;
    do *P = 0 ; while ((P = (IP4_HOOK**) ip4_next (w)) != 0) ;
    ip4_walk_close (w);

    /* now we can remove the table */
    p = l->iplist ;
    
    free (l) ;
    return p;
}

/* ----------------------------------------------------------------------- *
 * Range functions
 * ----------------------------------------------------------------------- */

/* loop over all ranges */
void for_known_range (RANGE                                           *r,
                      void (*cb) (struct in_addr, struct in_addr, void*),
                      void                                            *d) {
    if (r && cb) {
        int               n = r->entries ; /* at least 1 */
        struct _ip_range *p = r->ip ;
        do {
            struct in_addr from, to ;
            from.s_addr = htonl (p->from);
            to.s_addr   = htonl (p->to);
            (*cb) (from, to, d);
        }
        while (++ p, -- n > 0);
    }
}

unsigned known_address (RANGE          *r,
                        struct in_addr *a) {
    if (r) {
        ip4addr_t        ip = ip2ip4addr (a);
        int               n = r->entries ; /* at least 1 */
        struct _ip_range *p = r->ip ;
        do
            if (p->from <= ip && ip <= p->to) return 1;
        while (++ p, -- n > 0);
    }
    return 0;
}

/* ----------------------------------------------------------------------- *
 * Range Constructor/Destructor
 * ----------------------------------------------------------------------- */

RANGE *range_init (IP4_POOL *pool) {

    IP4_WALK *w = ip4_walk_init (pool, 0); /* pool == 0 is acceptable */

    int        n ;
    RANGE     *r ;

    if (w == 0)
        return 0 ;
    
    r  = XMALLOC (RANGE) ;
    n  = 0;  /* array index */

    r->ip [n].from = r->ip [n].to = ip4_inx2ip4addr (w);

    do {
        ip4addr_t ip = ip4_inx2ip4addr (w);
        if (r->ip [n].to + 1 < ip) {
            r = (RANGE*) xrealloc
                (r, sizeof (RANGE) + (++ n) * sizeof (struct _ip_range));
            r->ip [n].from = ip ;
        }
        r->ip [n].to = ip ;
    }
    while (ip4_next (w)) ;
    ip4_walk_close (w);

    r->entries = n + 1 ;
        
    return r;
}


void range_close (RANGE *r) {
    free (r) ;
}

/* ----------------------------------------------------------------------- *
 * mac/ip registry
 * ----------------------------------------------------------------------- */

REGISTER *register_init (IP4_POOL *ip4) {
    IP4_WALK *w = ip4_walk_init (ip4, 0);
    
    if (w) {
        IP4_SEEN **P;
        REGISTER *g;
        int n = 0 ;

        /* duplicate argument tree */
        ip4 = ip4_pool_merge (0, ip4);

        /* count number of entries, needed */
        do n ++ ; while (ip4_next (w)) ;
        ip4_walk_close (w);

        /* allocate and initialise register structure */
        g = (REGISTER*) xmalloc
            (sizeof (REGISTER) - sizeof (IP4_SEEN) + n * sizeof (IP4_SEEN));
        g->byip = ip4;

        /* set up leaves */
        w = ip4_walk_init (g->byip, 0);
        P = (IP4_SEEN**) ip4_this (w) ;
        do {
            *P = g->seen + g->entries ++ ;
            (*P)->ip4.s_addr = htonl (ip4_inx2ip4addr (w)) ;
        }
        while ((P = (IP4_SEEN**)ip4_next (w)) != 0);
        ip4_walk_close (w);

        return g;
    }

    return 0;
}


void register_close (REGISTER *g) {
    if (g) {
        ip4_pool_close (g->byip, 0);
        free (g);
    }
}

int max_registered (REGISTER *g) {
    return g->entries ;
}

int num_registered (REGISTER *g) {
    return g->used ;
}

int max_slot_registered (REGISTER *g) {
    int slot ;

    if (g->used)
        for (slot = g->entries - 1; slot >= 0; slot --)
            if (g->seen [slot].last_seen) return slot ;

    return -1;
}

int min_slot_registered (REGISTER *g) {
    int slot ;

    if (g->used)
        for (slot = 0; slot < g->entries; slot ++)
            if (g->seen [slot].last_seen) return slot ;

    return -1;
}

time_t find_slot_registered (REGISTER *g,
                             struct in_addr *ip,
                             struct ether_addr *mac,
                             int slot) {
    if (0 <= slot && slot < g->entries) {
        IP4_SEEN *p = g->seen + slot ;
        if (p->last_seen) {
            if (ip) ip->s_addr = p->ip4.s_addr ;
            if (mac) memcpy (mac, &p->mac, ETH_ALEN);
            return p->last_seen;
        }
    }
    
    return 0 ;
}

time_t register_ask (REGISTER            *g,
                     struct in_addr     *ip,
                     struct ether_addr *mac) {
    IP4_SEEN **P = (IP4_SEEN**) ip4_lookup (g->byip, ip);
    if (P &&
        (*P)->last_seen &&
        memcmp (&(*P)->mac, mac, sizeof (struct ether_addr)) == 0) {
        time_t last_seen = (*P)->last_seen ;
        (*P)->last_seen = time (0);
        return last_seen ;
    }
    return 0;
}


time_t register_set (REGISTER            *g,
                     struct in_addr     *ip,
                     struct ether_addr *mac) {
    IP4_SEEN **P = (IP4_SEEN**) ip4_lookup (g->byip, ip);
    if (P) {
        time_t last_seen = (*P)->last_seen ;
        if (mac) {
            memcpy (&(*P)->mac, mac, sizeof (struct ether_addr));
            (*P)->last_seen = time (0);
            if (last_seen == 0) {
                last_seen = 1 ;
                g->used ++ ;
            }
        }
        else if (last_seen) {
            (*P)->last_seen = 0;
            g->used -- ;
        }
        return last_seen ;
    }
    return 0;
}


/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
