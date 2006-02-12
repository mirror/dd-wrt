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
 * $Id: arp.c,v 1.26 2005/04/30 11:53:59 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <features.h>    /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
# include <netpacket/packet.h>
# include <net/ethernet.h>     /* the L2 protocols */
#else
# include <asm/types.h>
# include <linux/if_packet.h>
# include <linux/if_ether.h>   /* The L2 protocols */
#endif

#include "util.h"

#define __ARP_PRIVATE
#include "arp.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */

#ifdef _DEBUG
#define _ARP_DEBUG
#endif

static
const struct arphdr arpreq_hdr = { 
    H16TON (ARPHRD_ETHER),
    H16TON (ETHERTYPE_IP), 
    ETH_ALEN,
    IP4_ALEN,
    H16TON (ARPOP_REQUEST) 
};


static
const struct arphdr arply_hdr = { 
    H16TON (ARPHRD_ETHER),
    H16TON (ETHERTYPE_IP), 
    ETH_ALEN,
    IP4_ALEN,
    H16TON (ARPOP_REPLY) 
};

/* ----------------------------------------------------------------------- *
 * Public: arp reply ring buffer
 * ----------------------------------------------------------------------- */

static
void send_isat_reply (ARP                *arp,
                      struct in_addr     *sip,   /* unaligned */
                      struct in_addr     *dip,   /* unaligned */
                      struct ether_addr *smac,   /* unaligned */
                      struct ether_addr *dmac) { /* unaligned */
    struct ether_arp apkt ;
    IN_DEV *dev = arp->dev;
    struct sockaddr_ll sll;
   
#   ifdef _ARP_DEBUG
#   define LOGTAG(s)  "arp_reply: " s
#   else
#   define LOGTAG(s)  s
#   endif
    
    /* assemble arp reply header */
    memcpy (& apkt.ea_hdr, &arply_hdr, sizeof (apkt.ea_hdr));
  
    /* assemble arp reply: mac addresses */
    memcpy (& apkt.arp_sha, smac, ETH_ALEN);
    memcpy (& apkt.arp_tha, dmac, ETH_ALEN);
    
    /* assemble arp reply: ip addresses */
    memcpy (& apkt.arp_spa, sip, IP4_ALEN);
    memcpy (& apkt.arp_tpa, dip, IP4_ALEN);
    
    /* initialise socket address structure */
    memcpy (&sll.sll_addr, dmac, sizeof (sll.sll_addr));
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons (ETH_P_ARP);
    sll.sll_ifindex  = dev->ifindex;

    /* send that packet */
    if (sendto (dev->arp_fd, &apkt, sizeof (apkt), 0,
                (struct sockaddr *)&sll, sizeof (sll)) < 0) {
        logger (LOG_ERR, LOGTAG ("ERROR %s: %s (%s)"),
                ethaddr_pp ((struct ether_addr*)&sll.sll_addr),
                arpkt_pp (&apkt, verbosity & ARP_INBOUND_MORE),
                strerror (errno)) ;
    }
    else {
        /* log response packet */
        if (verbosity & ARP_REPLY_LOG)
            logger (LOG_INFO, LOGTAG ("replying to %s: %s"),
                    ethaddr_pp ((struct ether_addr*)&sll.sll_addr),
                    arpkt_pp (&apkt, verbosity & ARP_INBOUND_MORE)) ;
    }

#   undef LOGTAG
}


/* run through the arp reply batch queue */
void arp_reply_pending (ARP                *arp,
                        struct in_addr *sdelete) { /* unaligned */
    ARPLY *batch = arp->batch ;

    if (batch) {
        time_t now = time (0);
        do {
            ARPLY *ptr = batch->next ;

            /* delete the given source ip address when set */
            if (sdelete && memcmp (&ptr->sip, sdelete, IP4_ALEN) == 0)
                /* unconditionally delete this entry */
                ptr->repeat = 0;
            else {
                /* is it time to send this reply frame, at all ? */
                if (now < ptr->due) continue ;
                /* ok, send a reply frame */
                send_isat_reply (arp,
                                 &ptr->sip,  /* source ip */
                                 &ptr->dip,  /* target ip */
                                 &ptr->src,  /* source mac */
                                 &ptr->dst); /* target mac */
                ptr->due = now + arp->spoof_send_intv ;
                ptr->repeat -- ;
            }

            if (-- ptr->repeat <= 0) {         /* no more frames to send */
                if (ptr == batch) {
                    arp->batch = 0 ;           /* was the only record */
                    if (arp->scd)          /* tell the idle time sched */
                        (*arp->scd) (arp->scd_data, 0) ;
                }
                else
                    batch->next = ptr->next ;  /* unlink from queue */

                /* move it to the garbage queue */
                ptr->next = arp->garbage ;
                arp->garbage = ptr ;
                if (arp->batch == 0) break ;
            }
        } while ((batch = batch->next) != arp->batch);
    }
}


static
void isat_reply (ARP               *arp,
                 struct in_addr    *sip, /* unaligned */
                 struct in_addr    *dip, /* unaligned */
                 struct ether_addr *src, /* unaligned */
                 struct ether_addr *dst, /* unaligned */
                 int        num_replies) {

    /* send pending frames but not from the sip ip address */
    arp_reply_pending (arp, sip);
    
    /* add to the batch queue if there is more than one frame to send */
    if (num_replies > 1) {
        /* want queue entry */
        ARPLY *rply = arp->garbage ;
        if (rply) {
            /* ok, there is space in the queue, so add that entry */
            arp->garbage = rply->next ;
            if (arp->batch) {
                rply->next       = arp->batch->next ;
                arp->batch->next = rply ;
            }
            else {
                rply->next = rply ;       /* wrap around */
                if (arp->scd)
                    /* notify the idle time scheduler */
                    (*arp->scd) (arp->scd_data, arp->spoof_send_intv);
            }
            arp->batch = rply ;
            /* initialise data ... */
            rply->repeat = num_replies - 1;
            rply->due    = time (0) + arp->spoof_send_intv ;
            memcpy (&rply->sip, sip, IP4_ALEN);
            memcpy (&rply->dip, dip, IP4_ALEN);
            memcpy (&rply->src, src, ETH_ALEN);
            memcpy (&rply->dst, dst, ETH_ALEN);
        }
    }

    send_isat_reply (arp, sip, dip, src, dst);
}


/* ------------------------------------------------------------------------ *
 * Public: arp packet handler
 * ------------------------------------------------------------------------ */

/* run through the arp reply batch queue */
void arp_pending (ARP *arp) {
    arp_reply_pending (arp, 0) ;
}

/* Reads an ARP packet and answer it */
void arp_catch (ARP *arp) {

    IN_DEV *dev = arp->dev;
    struct ether_arp apkt ;

#   ifdef _ARP_DEBUG
#   define LOGTAG(s)  "arp_catch: " s
#   else
#   define LOGTAG(s)  s
#   endif

    struct sockaddr_ll sll;
    struct in_addr src_ip, trg_ip ;
    MAC_REC *mac_rec ;
    int n, arp_len ;

    /* get packet */
    if ((arp_len = recvfrom (dev->arp_fd,
                             & apkt,
                             sizeof (apkt),
                             0,
                             (struct sockaddr *) &sll,
                             (n = sizeof (sll), &n))) < 0) {
        if (errno == EAGAIN)
            goto just_polling;
        else
            goto arp_read_error;
    }

    if (arp_len < sizeof (apkt))
        goto drop_small_arp_frame;

    /* The ARP type must be an Ethernet WHOIS request. */
    if (memcmp (& apkt, &arpreq_hdr, sizeof (arpreq_hdr)) != 0)
        goto drop_arp_frame;

    /* read source ip and target ip into (properly aligned) variable */
    memcpy (&src_ip, & apkt.arp_spa, sizeof (src_ip));
    memcpy (&trg_ip, & apkt.arp_tpa, sizeof (trg_ip));
     
    /* Check whether an address in the local range is requested and
       we should spoof a reply packet */
    if (arp->local != 0 && known_address (arp->local, &trg_ip)) {

        /* We do not process queries targeting an excluded IP. This
           range contains also the LAN interface addresses */
        if (known_address (arp->ignore, &trg_ip))
            goto drop_arp_frame;

        /* Check for a zero config arp: "WHOIS <myself> TELL <me>"
           with the sender IP set to 0 */
        if (arp->dhcp &&
            (src_ip.s_addr == 0 || src_ip.s_addr == trg_ip.s_addr) &&
            register_ask (arp->dhcp, & trg_ip,
                          (struct ether_addr *) & apkt.arp_sha))
            /* ok, let him go */
            goto drop_arp_frame;
            
        /* log query packet */
        if (verbosity & ARP_REQUEST_LOG)
            logger (LOG_INFO, LOGTAG ("spoof request from %s: %s"), 
                    ethaddr_pp ((struct ether_addr*)&sll.sll_addr),
                    arpkt_pp (& apkt, verbosity & ARP_INBOUND_MORE));

        /* reply with the arp spoof mac address */
        isat_reply (arp,
                    & trg_ip,    & src_ip,
                    & arp->fake, (struct ether_addr*) & apkt.arp_sha,
                    arp->spoof_replies);
        return ;
    }

    /* Check for a zero config arp: "WHOIS <myself> TELL <me>" */
    if (src_ip.s_addr == 0 || src_ip.s_addr == trg_ip.s_addr)
        goto drop_arp_frame;
        
    /* check whether we are configured to register a new session */
    if (arp->pool) {

        /*  We do not register queries from an excluded IP source address.
            This range contains also the LAN interface addresses */
        if (known_address (arp->ignore, & src_ip))
            goto drop_arp_frame;
  
        /* log query packet */
        if (verbosity & ARP_REQUEST_LOG)
            logger (LOG_INFO, LOGTAG ("register %s: %s"), 
                    ethaddr_pp ((struct ether_addr*)&sll.sll_addr),
                    arpkt_pp (& apkt, 1));

        mac_rec = make_mac_address
            (arp->pool, & src_ip, (struct ether_addr*)&sll.sll_addr);
 
        if (mac_rec == 0)
            /* no more pool addresses and unknown mac address */
            goto error_arp_frame_register;

        /* reply with the interface mac address */
        isat_reply (arp,
                    & trg_ip,   & src_ip, 
                    & dev->mac, (struct ether_addr*) & apkt.arp_sha,
                    0);
        return ;
    }

    /* otherwise drop that frame */

 drop_arp_frame:
    if (verbosity & ARP_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping arp frame from %s: %s"),
                ethaddr_pp ((struct ether_addr*)&sll.sll_addr), 
                arpkt_pp (& apkt, verbosity & ARP_INBOUND_MORE)) ;
    return ; 

 drop_small_arp_frame:
    if (verbosity & ARP_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping small arp frame: size %d < %d"),
                arp_len, sizeof (apkt));
    return ;

 error_arp_frame_register:
    logger (LOG_INFO, LOGTAG ("ERROR %s: cannot register"),
            arpkt_pp (& apkt, verbosity & ARP_INBOUND_MORE));
    return ; 

 arp_read_error:
    logger (LOG_INFO, LOGTAG ("arp read error: %s"), strerror (errno));
    return ;

 just_polling:
    if (verbosity & ARP_INBOUND_POLL)
        logger (LOG_INFO, LOGTAG ("polling for arp frames ..."));
    return ;
    
#   undef LOGTAG
}


/* ----------------------------------------------------------------------- *
 * Constructor/Destructor
 * ----------------------------------------------------------------------- */

ARP *arp_init (IN_DEV           *idev,
               LOOKUP           *pool,
               RANGE          *ignore,
               RANGE           *local,
               REGISTER         *dhcp,
               struct ether_addr *mac,
               int          queue_len,
               void   (*cb)(void*,int),
               void          *cb_data) {

    ARP *a ;

    if (queue_len == 0)
        /* use default settings */
        queue_len = ARP_BATCH_DEFAULT_LENGTH ;
    else if (queue_len < 0)
        /* batch queue processing is disabled */
        queue_len = 0 ;
    
    a = (ARP*) xmalloc
        (sizeof (ARP) - sizeof (ARPLY) + queue_len * sizeof (ARPLY));

    a->dev     =   idev; /* interface descriptor */
    a->pool    =   pool; /* pool to assign addresses from */
    a->ignore  = ignore; /* ignore these ip addresses */
    a->local   =  local; /* arp spoof when asking for these addresses */
    a->dhcp    =   dhcp; /* registered dhcp clients */

    if (mac) {           /* arp spoof, attact by this address */
        a->fake = *mac;
    }
    else if (local) {
        unsigned int x, y, z ;
        hw6addr_t eth;
        struct timeval seed ;
        sscanf (ARP_SPOOF_DEFAULT_MAC, "%x:%x:%x", &x, &y, &z);
        eth = (((x << 8) | y) << 8) | z ; eth <<= 24 ;
        gettimeofday (&seed, 0);             /* weak random is ok here */
        srand (seed.tv_sec ^ seed.tv_usec) ; /* weak random is ok here */
        eth |= (rand () >> 4) & 0xffffff ;   /* weak random is ok here */
        hw6addr2ip (&a->fake, eth);
        logger (LOG_WARNING, "Using mac address %s for arp spoofing",
                ethaddr_pp (&a->fake));
    }

    if (queue_len > 0) {
        while (-- queue_len > 0)
            /* initialise garbage queue */
            a->batch_pool [queue_len-1].next = a->batch_pool + queue_len ;
        a->garbage         = a->batch_pool;
        a->spoof_replies   = ARP_SPOOF_REPLIES ;
        a->spoof_send_intv = ARP_SPOOF_REP_INTV ;
        a->scd             = cb ;
        a->scd_data        = cb_data ;
    }
    
    return a;
}

void               arp_close   (ARP *a) { if (a) free (a) ; }
IN_DEV            *arp_device  (ARP *a) { return a ? a->dev    : 0; }
LOOKUP            *arp_pool    (ARP *a) { return a ? a->pool   : 0; }
RANGE             *arp_exclude (ARP *a) { return a ? a->ignore : 0; }
RANGE             *arp_local   (ARP *a) { return a ? a->local  : 0; }
struct ether_addr *arp_mac     (ARP *a) { return a ? &a->fake  : 0; }

/* ----------------------------------------------------------------------- *
 * End
 * ----------------------------------------------------------------------- */
