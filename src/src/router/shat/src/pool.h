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
 * $Id: pool.h,v 1.9 2005/03/19 14:45:04 jordan Exp $
 */

#ifndef __SHAT_POOL_H
#define __SHAT_POOL_H

#include "util.h"

typedef
struct _ip4_pool {
#   ifdef __POOL_PRIVATE
    int entries ;                  /* number of used links */
    struct _ip4_pool *octet [32] ; /* is not really an octet but a sextet */
#   else
    char any ;
#   endif
} IP4_POOL ;


typedef
struct _mac_pool {
#   ifdef __POOL_PRIVATE
    int entries ;                  /* number of used links */
    struct _mac_pool *octet [64] ; /* is not really an octet but a sextet */
#   else
    char any ;
#   endif
} MAC_POOL ;


typedef
struct _ip4_walk {
#   ifdef __POOL_PRIVATE
    IP4_POOL *ip ;
    struct {                       /* convenient wrapper for bzero() */
        struct {
            IP4_POOL *ip ;
            unsigned inx ;
        } node [7];
    } cache ;
#   else
    char any ;
#   endif
} IP4_WALK ;


typedef
struct _mac_walk {
#   ifdef __POOL_PRIVATE
    MAC_POOL *mac ;
    struct {                       /* convenient wrapper for bzero() */
        struct {
            MAC_POOL *mac ;
            unsigned inx ;
        } node [8];
    } cache ;
#   else
    char any ;
#   endif
} MAC_WALK ;


extern IP4_POOL *ip4_pool_init  (void);
extern unsigned  ip4_pool_empty (IP4_POOL*);
extern IP4_POOL *ip4_pool_merge (IP4_POOL*, IP4_POOL*);
extern void      ip4_pool_close (IP4_POOL*, void(*)(void*));
extern void    **ip4_fetch      (IP4_POOL*, const struct in_addr*);
extern void    **ip4_lookup     (IP4_POOL*, const struct in_addr*);

extern MAC_POOL *mac_pool_init  (void);
extern void      mac_pool_close (MAC_POOL*, void(*)(void*));
extern void     *mac_delete     (MAC_POOL*, const struct ether_addr*);
extern void    **mac_fetch      (MAC_POOL*, const struct ether_addr*);
extern void    **mac_lookup     (MAC_POOL*, const struct ether_addr*);

extern MAC_WALK *mac_walk_init (MAC_POOL*, MAC_WALK*);
extern void      mac_walk_close           (MAC_WALK*);
extern void     **mac_this                (MAC_WALK*);
extern void     **mac_next                (MAC_WALK*);

extern IP4_WALK *ip4_walk_init (IP4_POOL*, IP4_WALK*);
extern void      ip4_walk_close           (IP4_WALK*);
extern ip4addr_t ip4_inx2ip4addr          (IP4_WALK*);
extern void    **ip4_this                 (IP4_WALK*);
extern void    **ip4_next                 (IP4_WALK*);

#endif /* __SHAT_POOL_H */
