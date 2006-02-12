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
 * $Id: lookup.h,v 1.19 2005/04/30 12:00:21 jordan Exp $
 */

#ifndef __SHAT_LOOKUP_H
#define __SHAT_LOOKUP_H

#include "pool.h"
#include "device.h"
#include "util.h"

// /* defined in util.h */
// #define DEFAULT_REC_LEASE  100 /* timeout since last visit */

typedef
struct _mac_rec {
    struct in_addr     real_ip;	/* cl. ip in network order */
    struct ether_addr real_mac; /* mac address */
    struct _ip4_rec   *ip4_rec; /* reference to the ip address */
    time_t            time2die;	/* record life time indicator */
} MAC_REC ;

typedef
struct _ip4_rec {
    struct in_addr     pool_ip;	/* cl. ip in network order */
    MAC_REC           *mac_rec; /* reference to the mac address */
} IP4_REC ;


#ifdef __LOOKUP_PRIVATE
typedef
struct _mac_hook {
    MAC_REC            mac_rec; /* production data */
    char                  busy; /* activation or garbage flag */
} MAC_HOOK ;

typedef
struct _ip4_hook {
    IP4_REC            ip4_rec; /* production data */
    char                  busy; /* activation or garbage flag */
} IP4_HOOK ;
#endif /* __LOOKUP_PRIVATE */


typedef
struct _lookup {
    unsigned     mac_rec_lease; /* timeout since last mac record visit */
#   ifdef __LOOKUP_PRIVATE
    IN_DEV                *dev; /* need to update the arp table */
    MAC_POOL          *maclist; /* list of seen mac addresses */
    IP4_POOL           *iplist; /* indexed assign pool access */

    /* data records for use with the ip list */
    int              pool_free; /* num of free entries in the assign pool */
    int              pool_size; /* total number of pool entries */
    IP4_HOOK       *pool_cache; /* register the last garbage record */
    IP4_HOOK     pool_recs [1]; /* pool to assign addresses from */
    /* open end */
#   endif /* __LOOKUP_PRIVATE */
} LOOKUP ;


typedef
struct _ip4_seen {
    struct ether_addr      mac; /* to be accessed by the mac address */
    struct in_addr         ip4; /* corresponding ip address */
    time_t           last_seen;	/* record life time indicator */
} IP4_SEEN ;


typedef
struct _register {              /* mac-ip assignment register */
#   ifdef __LOOKUP_PRIVATE
    IP4_POOL             *byip; /* indexed assign pool access */
    int                   used; /* number of registered records */
    int                entries; /* max number of records, following */
    IP4_SEEN          seen [1]; /* to be accessed by the mac address */
#   endif /* __LOOKUP_PRIVATE */
} REGISTER ;


typedef
struct _range {
#   ifdef __LOOKUP_PRIVATE
    int entries ;
    struct _ip_range {
        ip4addr_t from, to ; /* ip address interval, host order */
    } ip [1];
    /* open end */
#   else
    char any ;
#   endif /* __LOOKUP_PRIVATE */
} RANGE ;


extern REGISTER    *register_init  (IP4_POOL*);
extern void         register_close (REGISTER*);
extern time_t       register_ask   (REGISTER*,
                                    struct    in_addr*,
                                    struct ether_addr*);
extern time_t       register_set   (REGISTER*,
                                    struct    in_addr*,
                                    struct ether_addr*);
extern time_t find_slot_registered (REGISTER*,
                                    struct in_addr*,
                                    struct ether_addr*,
                                    int);
extern int          max_registered (REGISTER*);
extern int          num_registered (REGISTER*);
extern int     min_slot_registered (REGISTER*);
extern int     max_slot_registered (REGISTER*);

extern void for_registered (REGISTER*, int first_slot, int num,
                            void(*)(struct in_addr,struct ether_addr,void*),
                            void*);

extern RANGE    *range_init (IP4_POOL*);
extern void      range_close   (RANGE*);
extern unsigned  known_address (RANGE*, struct in_addr*);
extern void  for_known_range   (RANGE*,
                                void(*)(struct in_addr,struct in_addr,void*),
                                void*);

extern LOOKUP   *lookup_init (IP4_POOL*, IN_DEV*);
extern IP4_POOL *lookup_close (LOOKUP*);

extern unsigned purge_pool_garbage (LOOKUP*);
extern     void  free_pool_address (LOOKUP*, IP4_REC*);
extern     void   for_pool_address (LOOKUP*, int,
                                    void(*) (IP4_REC*, void*),
                                    void*);

extern    int     get_slot_number (LOOKUP*,IP4_REC*);
extern    int     max_slot_used   (LOOKUP*);
extern    int     min_slot_used   (LOOKUP*);
extern    int     num_slots       (LOOKUP*);
extern    int     num_slots_used  (LOOKUP*);

extern   void refresh_mac_address (LOOKUP*,MAC_REC*);
extern IP4_REC *find_pool_address (LOOKUP*, struct    in_addr*);
extern IP4_REC *find_pool_slot    (LOOKUP*, int);
extern IP4_REC  *use_pool_slot    (LOOKUP*, int);
extern MAC_REC  *find_mac_address (LOOKUP*, struct ether_addr*);
extern MAC_REC  *make_mac_address (LOOKUP*,
                                   struct    in_addr*,
                                   struct ether_addr*);

#endif /* __SHAT_LOOKUP_H */
