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
 * $Id: comhandler.c,v 1.8 2005/05/29 15:02:26 jordan Exp $
 */

#ifndef DISCARD_CMD_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>

#include "util.h"
#include "ctrl.h"

#define __DEVICE_PRIVATE
#include "device.h"

#define __COMHANDLER_PRIVATE
#include "comhandler.h"

/* ----------------------------------------------------------------------- *
 * Private: Communication handler support
 * ----------------------------------------------------------------------- */

static
void reply_rec (COMM *o, MAC_REC *r, int pfx) {
    ctrl_reply (o->ctrl, "%s%s %s %s %d\n",
                pfx ? "+OK: " : "",
                ethaddr_pp (&r->real_mac),
                ipaddr_pp (&r->real_ip),
                ipaddr_pp (&r->ip4_rec->pool_ip),
                get_slot_number (o->lookup, r->ip4_rec));
}

static
void show_exclude_range (struct in_addr f,
                         struct in_addr t,
                         void          *p) {
    COMM *o = p;
    if (!o->visited ++) ctrl_reply (o->ctrl, "arp exclude range =");
    ctrl_reply (o->ctrl, " [%s, %s]", ipaddr_pp (&f), ipaddr_pp (&t));
}

static
void show_local_range (struct in_addr f,
                       struct in_addr t,
                       void          *p) {
    COMM *o = p;
    if (!o->visited ++) ctrl_reply (o->ctrl, "arp scan protection =");
    ctrl_reply (o->ctrl, " [%s, %s]", ipaddr_pp (&f), ipaddr_pp (&t));
}

static
void show_exlocal_range (struct in_addr f,
                         struct in_addr t,
                         void          *p) {
    COMM *o = p;
    if (!o->visited ++) ctrl_reply (o->ctrl, "ip exclude range =");
    ctrl_reply (o->ctrl, " [%s, %s]", ipaddr_pp (&f), ipaddr_pp (&t));
}


/* ----------------------------------------------------------------------- *
 * Public: Communication handlers
 * ----------------------------------------------------------------------- */

COMM *comm_init (void) {
    return XMALLOC (COMM);
}

COMM *comm_update (COMM     *o,
                   LOOKUP   *l,
                   ARP      *a,
                   IP2E     *i,
                   CTRL     *c,
                   REGISTER *r) {
    o->started = time (0);
    o->ip2e   = i ;
    o->arp    = a ;
    o->lookup = l ;
    o->ctrl   = c ;
    o->reg    = r ;
    return o;
}


void comm_info (void *state, int argc, const char **argv) {

    COMM *o = state;
    IN_DEV *i ;
    TUN_DEV *t;
    struct ether_addr *e ;
    const char *me = argv [0];
    int n, limit ;

    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc > 1) goto usage;

    ctrl_reply (o->ctrl, "+OK:\n");

    n = time (0) - o->started ;
    ctrl_reply (o->ctrl, "uptime: ");
    if (n > 60) {
        int min =   n / 60 ;
        int h   = min / 60 ;
        if (h > 24) {
            ctrl_reply (o->ctrl, "%dd ", h / 24);
            h %= 24 ;
        }
        ctrl_reply (o->ctrl, "%d:%02d:%02d\n", h, min % 60, n % 60);
    }
    else
        ctrl_reply (o->ctrl, "%ds\n", n);
        
    if ((i = ip2ether_device (o->ip2e)) != 0) {
        /* show lan device settings */
        ctrl_reply (o->ctrl, "lan device name = %s(%u)\n",
                    i->name, i->ifindex);
        
        ctrl_reply (o->ctrl, "lan ip address = %s/%s\n",
                    ipaddr_pp (&i->ipa), ipaddr_pp (&i->ipm));
        
        ctrl_reply (o->ctrl, "lan device mac address = %s\n",
                    ethaddr_pp (&i->mac));
    }

#   ifdef __REDUNDANT_HERE__
    if ((i = arp_device (o->arp)) != 0) {
        /* show arp device */
        ctrl_reply (o->ctrl, "arp device name = %s(%u)\n",
                    i->name, i->ifindex);

        ctrl_reply (o->ctrl, "arp device ip address = %s/%s\n",
                    ipaddr_pp (&i->ipa), ipaddr_pp (&i->ipm));

        ctrl_reply (o->ctrl, "arp device mac address = %s\n",
                    ethaddr_pp (&i->mac));
    }
#   endif /* __REDUNDANT_HERE__ */

    if ((e = arp_mac (o->arp)) != 0)
        ctrl_reply (o->ctrl, "arp spoof mac address = %s\n",
                    ethaddr_pp (e));

    if ((t = ip2ether_tundev (o->ip2e)) != 0) {
        /* show tun/shat device settings */
        ctrl_reply (o->ctrl, "tunnel device name = %s(%u)\n",
                    t->name, t->ifindex);
        ctrl_reply (o->ctrl, "tunnel ip address = %s\n",
                    ipaddr_pp (&t->ipa));
    }

    o->visited = 0;
    for_known_range (arp_exclude      (o->arp),  show_exclude_range, o);
    if (o->visited) { ctrl_reply (o->ctrl, "\n"); o->visited = 0; }

    for_known_range (arp_local        (o->arp),  show_local_range,   o);
    if (o->visited) { ctrl_reply (o->ctrl, "\n"); o->visited = 0; }

    for_known_range (ip2ether_exclude (o->ip2e), show_exlocal_range, o);
    if (o->visited) ctrl_reply (o->ctrl, "\n");

    if (o->lookup && (limit = num_slots (o->lookup)) != 0) {
        struct in_addr ip, from = use_pool_slot (o->lookup, 0)->pool_ip;
        ip4addr_t            to = ip2ip4addr (&from);
        ctrl_reply (o->ctrl, "pool address range =");
        for (n = 1; n < limit; n ++) {
            ip = use_pool_slot (o->lookup, n)->pool_ip;
            if (++ to == ip2ip4addr (&ip)) continue ;
            ctrl_reply   /* print address range */
                (o->ctrl, " [%s, %s]", ipaddr_pp (&from), ip4addr_pp (to-1));
            from = ip ;  /* start new interval */
            to = ip2ip4addr (&ip);
        }
        ctrl_reply   /* print address range */
            (o->ctrl, " [%s, %s]\n", ipaddr_pp (&from), ip4addr_pp (to));
    }

    return;
 
 usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
 help:
    ctrl_reply (o->ctrl, "%s -- print setup info\n", me);
}


/* lookup function */
void comm_find_mac (void *state, int argc, const char **argv) {

    COMM *o = state;
    const char *me = argv [0];
    struct ether_addr *mac;
    
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if ((mac = ether_aton (argv [1])) != 0) {
        MAC_REC *rec = find_mac_address (o->lookup, mac);

        if (rec == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else
            reply_rec (o, rec, 1);
        return ;
    }

usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <mac> -- client by mac address\n", me);
}



void comm_find_ip (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    struct in_addr ip;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if (inet_aton (argv [1], &ip) != 0) {
        IP4_REC *rec = find_pool_address (o->lookup, &ip);
        if (rec == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else
            reply_rec (o, rec->mac_rec, 1);
        return ;
    }

usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <ip> -- client by mapped ip\n", me);
}



void comm_delete_ip (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    struct in_addr ip;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if (inet_aton (argv [1], &ip) != 0) {
        IP4_REC *rec = find_pool_address (o->lookup, &ip);
        if (rec == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else {
            free_pool_address (o->lookup, rec);
            ctrl_reply (o->ctrl, "-OK: delete %s\n",
                        ethaddr_pp (&rec->mac_rec->real_mac));

        }
        return ;
    }

usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <ip> -- delete client ip\n", me);
}


void comm_find_slot (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    IP4_REC *rec ;
    unsigned n;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if (integer (&n, argv [1])) {
        if ((rec = find_pool_slot (o->lookup, n)) == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else
            reply_rec (o, rec->mac_rec, 1);
        return ;
    }
    
usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <slot> -- client by busy slot\n", me);
}


void comm_delete_slot (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    IP4_REC *rec ;
    unsigned n;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if (integer (&n, argv [1])) {
        if ((rec = find_pool_slot (o->lookup, n)) == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else {
            free_pool_address (o->lookup, rec);
            ctrl_reply (o->ctrl, "-OK: delete %s\n",
                        ethaddr_pp (&rec->mac_rec->real_mac));
        }
        return ;
    }
    
usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <slot> -- delete client slot\n", me);
}


void comm_ping_slot (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    IP4_REC *rec ;
    unsigned n;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }
    if (argc == 1) goto usage;

    if (integer (&n, argv [1])) {
        if ((rec = find_pool_slot (o->lookup, n)) == 0)
            ctrl_reply (o->ctrl, "-ERR: %s %s -- not found\n", me, argv [1]);
        else {
            ip2ether_ping (o->ip2e, rec->mac_rec);
            ctrl_reply (o->ctrl, "-OK: ping %s\n",
                        ethaddr_pp (&rec->mac_rec->real_mac));
        }
        return ;
    }
    
usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply (o->ctrl, "%s <slot> -- send ping to client slot\n", me);
}


void comm_list (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    IP4_REC *rec ;
    unsigned first, num;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }

    if (argc < 2) {
        ctrl_reply (o->ctrl, "+OK: %d %d %d %d",
                    num_slots_used (o->lookup),
                    num_slots      (o->lookup),
                    min_slot_used  (o->lookup),
                    max_slot_used  (o->lookup));
        return;
    }

    if (argc < 3)
        num = CTRL_IOBUF_SIZE / 80 ;
    else 
        if (integer (&num, argv [2]) == 0) goto usage ;
                      
    if (integer (&first, argv [1])) {
        /* adjust the number of slots that can be listed */
        int used = num_slots_used (o->lookup) ;
        int max  = max_slot_used  (o->lookup) ;

        /* the answer should fit into the io buffer */
        if (num > CTRL_IOBUF_SIZE / 80) num = CTRL_IOBUF_SIZE / 80 ;
        if (num > used) num = used ;

        /* adjust first entry */
        if (first < 5) {
            int start = min_slot_used (o->lookup);
            if (first < start)
                first = start ;
        }

        /* list records */
        ctrl_reply (o->ctrl, "+OK: %d\n", num);
        while (num > 0 && first <= max) {
            if ((rec = find_pool_slot (o->lookup, first ++)) == 0)
                continue ;
            num -- ;
            reply_rec (o, rec->mac_rec, 0);
        }
        return ;
    }
    
usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply
        (o->ctrl, "%s [<first-slot> [how-many]] -- list client slots\n", me);
}



void comm_reg_list (void *state, int argc, const char **argv) {
    COMM *o = state;
    const char *me = argv [0];
    unsigned first, num;
        
    if (me [0] == '?' && !me [1]) {me = argv [1]; goto help; }

    if (argc < 2) {
        ctrl_reply (o->ctrl, "+OK: %d %d %d %d",
                    num_registered      (o->reg),
                    max_registered      (o->reg),
                    min_slot_registered (o->reg),
                    max_slot_registered (o->reg));
        return;
    }

    if (argc < 3)
        num = CTRL_IOBUF_SIZE / 80 ;
    else 
        if (integer (&num, argv [2]) == 0) goto usage ;
                      
    if (integer (&first, argv [1])) {
        /* adjust the number of slots that can be listed */
        int used  = num_registered      (o->reg) ;
        int max   = max_slot_registered (o->reg) ;
        
        /* the answer should fit into the io buffer */
        if (num > CTRL_IOBUF_SIZE / 80) num = CTRL_IOBUF_SIZE / 80 ;
        if (num > used) num = used ;

        /* adjust first entry */
        if (first < 5) {
            int start = min_slot_registered (o->reg) ;
            if (first < start)
                first = start ;
        }
        
        /* list records */
        ctrl_reply (o->ctrl, "+OK: %d", num);
        while (num > 0 && first <= max) {
            struct in_addr     ip;
            struct ether_addr mac;
            time_t seen = find_slot_registered (o->reg, &ip, &mac, first);
            if (seen == 0) {
                first ++ ;
                continue ;
            }
            num -- ;
            ctrl_reply (o->ctrl, "\n%s %s %s %d",
                        ethaddr_pp (&mac),
                        ipaddr_pp (&ip),
                        time_pp (seen),
                        first ++);
        }
        
        return ;
    }
    
usage:
    ctrl_reply (o->ctrl, "-ERR: usage: ");
help:
    ctrl_reply
        (o->ctrl, "%s [<first-slot> [how-many]] -- list arp slots\n", me);
}

#endif /* DISCARD_CMD_SOCKET */

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
