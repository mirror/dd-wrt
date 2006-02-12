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
 * $Id: pool.c,v 1.9 2005/03/19 14:45:04 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "util.h"

#define __POOL_PRIVATE
#include "pool.h"

/* ----------------------------------------------------------------------- *
 * local defs
 * ----------------------------------------------------------------------- */

typedef
struct _leaf_entry {
    void *seen ;
} LEAF_ENTRY ;

/* ----------------------------------------------------------------------- *
 * local helpers
 * ----------------------------------------------------------------------- */

static
unsigned ip4_walk_continue (int     inx,  /* starting node */
                            IP4_POOL *p,
                            IP4_WALK *w) {
    int n ;

    if (inx < 0)
        return 0 ;

    /* continue with an intermediate node link */
    n = w->cache.node [inx].inx ;
    
    do {
        while (p->octet [n] == 0)
            if (++ n > 0x1f) return 0 ;
        w->cache.node [inx].ip  = p ;
        w->cache.node [inx].inx = n ;    /* remember that node */
        p = p->octet [n] ;               /* step on */
        n = 0 ;                          /* start at the zero link */
    }
    while (++ inx < 7); /* 7 nodes with 5 bits => 35 bits (32 only needed) */

    return 1;
}


static
void ip4_pool_node_close (IP4_POOL *p, int depth, void(*cb)(void*)) {

    if (depth < 7) { /* 7 nodes with 5 bits => 35 bits (32 only needed) */
        int n = 0 ;
        do
            if (p->octet [n])
                ip4_pool_node_close (p->octet [n], depth + 1, cb);
        while (++ n <= 0x1f); 
    }
    else if (cb && ((LEAF_ENTRY*)p)->seen)
        (*cb) (((LEAF_ENTRY*)p)->seen) ;

    free (p) ;
}


static
unsigned mac_walk_continue (int     inx,  /* starting node */
                            MAC_POOL *m,
                            MAC_WALK *w) {
    int n ;

    if (inx < 0)
        return 0 ;

    /* continue with an intermediate node link */
    n = w->cache.node [inx].inx ;
    
    do {
        while (m->octet [n] == 0)
            if (++ n > 0x3f) return 0 ;
        w->cache.node [inx].mac = m ;
        w->cache.node [inx].inx = n ;    /* remember that node */
        m = m->octet [n] ;               /* step on */
        n = 0 ;                          /* start at the zero link */
    }
    while (++ inx < 8); /* 8 nodes with 6 bits => 48 bits */

    return 1;
}


static
void mac_pool_node_close (MAC_POOL *p, int depth, void(*cb)(void*)) {

    if (depth < 8) {  /* 8 nodes with 6 bits => 48 bits */
        int n = 0 ;
        do
            if (p->octet [n])
                mac_pool_node_close (p->octet [n], depth + 1, cb);
        while (++ n <= 0x3f);
    }
    else if (cb && ((LEAF_ENTRY*)p)->seen)
        (*cb) (((LEAF_ENTRY*)p)->seen) ;
    
    free (p) ;
}

/* ----------------------------------------------------------------------- *
 * Public: Constructors/destructors
 * ----------------------------------------------------------------------- */

IP4_POOL *ip4_pool_init (void) {
    return XMALLOC (IP4_POOL);
}

void ip4_pool_close (IP4_POOL *p, void(*cb)(void*)) {
    if (p) ip4_pool_node_close (p, 0, cb);
}

MAC_POOL *mac_pool_init (void) {
    return XMALLOC (MAC_POOL);
}

void mac_pool_close (MAC_POOL *p, void(*cb)(void*)) {
    if (p) mac_pool_node_close (p, 0, cb);
}

/* reset or create the walk register */
IP4_WALK *ip4_walk_init (IP4_POOL *p,
                         IP4_WALK *w) {
    IP4_WALK me ;

    if (w == 0) {
        if (p == 0) return 0;
        w = &me ;
    }

    XZERO (w->cache);
    
    /* set up to the first entry */
    if (ip4_walk_continue (0, p, w) == 0)
        return 0 ;
    
    w->ip = p ;
    if (w == &me) w = xmemdup (&me);
    return w;
}

void ip4_walk_close (IP4_WALK *w) {
    if (w) free (w);
}

/* reset or create the walk register */
MAC_WALK *mac_walk_init (MAC_POOL *m,
                         MAC_WALK *w) {
    MAC_WALK me ;

    if (w == 0) {
        if (m == 0) return 0;
        w = &me ;
    }

    XZERO (w->cache);
    
    /* set up to the first entry */
    if (mac_walk_continue (0, m, w) == 0)
        return 0 ;
    
    w->mac = m ;
    if (w == &me) w = xmemdup (&me);
    return w;
}

void mac_walk_close (MAC_WALK *w) {
    if (w) free (w);
}

/* ----------------------------------------------------------------------- *
 * Public: IP lookup, add, remove entry
 * ----------------------------------------------------------------------- */

/* create entry unless present */
void **ip4_fetch (IP4_POOL             *p,
                  const struct in_addr *q) { /* unaligned */
    IP4_POOL **R ;
    ip4addr_t ip = ip2ip4addr (q);
    int        i = 6 ; /* 7 nodes with 5 bits => 35 bits (32 only needed) */

    do {
		R = (*p).octet + ((ip >> (i * 5)) & 0x1f);
        if (*R == 0) {
			*R = XMALLOC (IP4_POOL);
            p->entries ++ ;
        }
        p = *R ;
    }
    while (-- i >= 0) ;
    
    if (p == 0)
        *(LEAF_ENTRY**)R = XMALLOC (LEAF_ENTRY);
    return & (*(LEAF_ENTRY**)R)->seen ;
}



/* lookup or die (return NULL) */
void **ip4_lookup (IP4_POOL             *p,
                   const struct in_addr *q) { /* unaligned */
    ip4addr_t ip = ip2ip4addr (q);
	int        i = 6 ; /* 7 nodes with 5 bits => 35 bits (32 only needed) */
    do
        if ((p = p->octet [(ip >> (i * 5)) & 0x1f]) == 0) return 0;
    while (-- i >= 0);
    return & ((LEAF_ENTRY*)p)->seen ;
}


unsigned ip4_pool_empty (IP4_POOL *p) {
    return p == 0 || p->entries == 0;
}

/* get current entry */
void **ip4_this (IP4_WALK *w) {
    IP4_POOL *p = w->cache.node [6].ip ;
    if (p == 0) return 0;
    return & ((LEAF_ENTRY*)(p->octet [w->cache.node [6].inx]))->seen ;
}


/* get the index of the current entry (host notation) */
ip4addr_t ip4_inx2ip4addr (IP4_WALK *w) {
    ip4addr_t  ip = 0 ;
    int         i = 6 ; /* 7 nodes with 5 bits => 35 bits (32 only needed) */
    do
        ip |= (w->cache.node [6 - i].inx & 0x1f) << (i * 5);
    while (-- i >= 0);
    return ip ;
}


/* walk that entry */
void **ip4_next (IP4_WALK *w) {

    int i = 6 ; /* 7 nodes with 5 bits => 35 bits (32 only needed) */

    /* advance one node link */
    while (++ w->cache.node [i].inx > 0x1f)
        if (-- i < 0) goto reset_return ;     /* step back one node */

    /* find next entry */
    while (ip4_walk_continue (i, w->cache.node [i].ip, w) == 0)
        do                                    /* not found */
            if (-- i < 0) goto reset_return ; /* step back one node */
        while (++ w->cache.node [i].inx > 0x1f) ;

    return ip4_this (w);

    /*NOTREACHED*/

reset_return:
    XZERO (w->cache);                         /* reset, initialise */
    ip4_walk_continue (0, w->ip, w);          /* start from the beginning */
    return 0 ;
}

/* merge the second range into the first */
IP4_POOL *ip4_pool_merge (IP4_POOL *p,
                          IP4_POOL *merge) {
    if (merge) {
        IP4_WALK *w = ip4_walk_init (merge, 0);
        if (p == 0)
            p = ip4_pool_init ();
        struct in_addr ip ;
        do {
            ip.s_addr = htonl (ip4_inx2ip4addr (w));
            ip4_fetch (p, &ip);
        } while (ip4_next (w)) ;
        ip4_walk_close (w);
    }

    return p;
}

/* ----------------------------------------------------------------------- *
 * Public: MAC lookup, add, remove entry
 * ----------------------------------------------------------------------- */

/* create entry unless present */
void **mac_fetch (MAC_POOL                *p,
                  const struct ether_addr *q) {  /* unaligned */
    MAC_POOL  **R ;
    hw6addr_t mac = mp2hw6addr (q);
	int         i = 7 ; /* 8 nodes with 6 bits => 48 bits */

    do {
		R = (*p).octet + ((mac >> (i * 6)) & 0x3f);
        if (*R == 0) {
			*R = XMALLOC (MAC_POOL);
            p->entries ++ ;
        }
        p = *R ;
	}
    while (-- i >= 0) ;

    if (p == 0)
        *(LEAF_ENTRY**)R = XMALLOC (LEAF_ENTRY);
    return & (*(LEAF_ENTRY**)R)->seen ;
}



/* lookup or die (return NULL) */
void **mac_lookup (MAC_POOL                *p,
                   const struct ether_addr *q) {  /* unaligned */
    hw6addr_t mac = mp2hw6addr (q);
	int         i = 7 ; /* 8 nodes with 6 bits => 48 bits */
    do
		if ((p = p->octet [(mac >> (i * 6)) & 0x3f]) == 0) return 0;
    while (-- i >= 0) ;
    return & ((LEAF_ENTRY*)p)->seen ;
}



/* lookup or die (return NULL) */
void *mac_delete (MAC_POOL                *p,
                  const struct ether_addr *q) {  /* unaligned */

    void        *l ;
    MAC_POOL  **R [8];

    MAC_POOL   *r = p;
    hw6addr_t mac = mp2hw6addr (q);
	int         i = 7 ; /* 8 nodes with 6 bits => 48 bits */

    do { /* record leaf node path in R [7..0] */
		R [i] = (*r).octet + ((mac >> (i * 6)) & 0x3f);
        if (*R [i] == 0)
            return 0 ;
        r = *R [i];
	}
    while (-- i >= 0) ;

    assert (r != 0);             /* otherwise the tre is corrupt */
    l = ((LEAF_ENTRY*)r)->seen ; /* return this value */

    i = 1 ;
    do {
        free (r);                /* remove empty nodes */
        *R [i - 1] = 0 ;
        r = *R [i] ;
    }
    while (++ i < 8 && -- r->entries == 0) ;

    return l;
}


/* get current entry */
void **mac_this (MAC_WALK *w) {
    MAC_POOL *m = w->cache.node [7].mac ;
    if (m == 0) return 0;
    return & ((LEAF_ENTRY*)(m->octet [w->cache.node [7].inx]))->seen ;
}


/* walk that entry */
void **mac_next (MAC_WALK *w) {

    int i = 7 ; /* 8 nodes with 6 bits => 48 bits */

    /* advance one node link */
    while (++ w->cache.node [i].inx > 0x3f)
        if (-- i < 0) goto reset_return ;     /* step back one node */

    /* find next entry */
    while (mac_walk_continue (i, w->cache.node [i].mac, w) == 0)
        do                                    /* not found */
            if (-- i < 0) goto reset_return ; /* step back one node */
        while (++ w->cache.node [i].inx > 0x3f) ;

    return mac_this (w);

    /*NOTREACHED*/

reset_return:
    XZERO (w->cache);                         /* reset, initialise */
    mac_walk_continue (0, w->mac, w);         /* start from the beginning */
    return 0 ;
}

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
