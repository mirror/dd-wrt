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
 * $Id: ip2ether.c,v 1.28 2005/04/30 11:48:22 jordan Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
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

#define __IP2ETHER_PRIVATE
#include "ip2ether.h"

#include "util.h"
#include "lookup.h"

/* ----------------------------------------------------------------------- *
 * variables and definitions
 * ----------------------------------------------------------------------- */

#ifdef           _DEBUG
#define _IP2ETHER_DEBUG
#endif

static const unsigned char ffff [] = {-1,-1,-1,-1,-1,-1,-1,-1};

static const unsigned char dhrply_hdr [] = {
    2,        /* op:    2 == reply */
    1,        /* htype: 1 == ethernet */
    ETH_ALEN, /* hlen:  6 == ETH_ALEN */
    0};       /* hops:  0 */

static const unsigned char dhcookie [] = {99, 130, 83, 99}; /* rfc 1497 */

/* ----------------------------------------------------------------------- *
 * Ping: send icmp/echo request over mac
 * ----------------------------------------------------------------------- */

void ip2ether_ping (IP2E *ip2e, MAC_REC *rec) {
    IN_DEV  *idev = ip2e->idev ;

    struct sockaddr_ll sll;
    char ping [sizeof (struct ethhdr) +
               sizeof (struct ip)     +
               sizeof (struct icmphdr)] ;

#   define EPKTlen sizeof (ping)
#   define EPKTptr ((struct ethhdr*)ping)
#   define EPKT(x) ((EPKTptr)->x)
#   define IPKTptr ((struct ip*)(EPKTptr+1))
#   define IPKTlen (sizeof (struct iphdr) + sizeof (struct icmphdr))
#   define IPKT(x) ((IPKTptr)->x)
#   define MPKTptr ((struct icmphdr*)(IPKTptr+1))
#   define MPKT(x) ((MPKTptr)->x)
#   define LOGTAG(s)  "ping: " s

    XZERO (ping);
    
    /* provide link layer information */
    EPKT (h_proto) = htons (ETH_P_IP) ;
    memcpy (EPKT (h_source),   &idev->mac, ETH_ALEN);
    memcpy (EPKT (h_dest), &rec->real_mac, ETH_ALEN);

    /* fill in ip frame */
    IPKT (ip_hl)  = 5 ;
    IPKT (ip_v)   = 4 ;
    IPKT (ip_id)  = (rand () >> 4) & 0xffff ;  /* weak random ok */ ;
    IPKT (ip_len) = htons (IPKTlen) ;
    IPKT (ip_ttl) = 2 ;
    IPKT (ip_p)   = IPPROTO_ICMP ;
    memcpy (&IPKT (ip_src), &idev->ipa,    sizeof (struct in_addr)) ;
    memcpy (&IPKT (ip_dst), &rec->real_ip, sizeof (struct in_addr)) ;

    /* and calculate ip checksum */
    IPKT (ip_sum) = ip_cksum (IPKTptr) ;

    /* assemble echo request */
    MPKT(type)             = ICMP_ECHO ;
    MPKT(code)             = 0 ;
    MPKT(un.echo.id)       = (rand () >> 4) & 0xffff ;  /* weak random ok */
    MPKT(un.echo.sequence) = (rand () >> 4) & 0xffff ;  /* weak random ok */
    
    MPKT (checksum) = icmp_cksum (IPKTptr, IPKTlen) ;

    /* send ethernet packet to the client */
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons (ETH_P_IP);
    sll.sll_ifindex  = idev->ifindex ;
    memcpy (sll.sll_addr, &rec->real_mac, sizeof (sll.sll_addr)) ;

    if (sendto (idev->eth_fd, EPKTptr, EPKTlen, 0, 
                (struct sockaddr*)&sll, sizeof (sll)) < 0) {
        logger (LOG_INFO, LOGTAG ("ERROR %s: %s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE),
                strerror (errno));
        return ;
    }
    
#   ifdef _IP2ETHER_DEBUG
    if (verbosity & IP2E_OUTBOUND_LOG)
        logger (LOG_INFO, LOGTAG (">> %s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE));
#   else
    if (verbosity & IP2E_OUTBOUND_LOG)
        logger (LOG_INFO, LOGTAG ("%s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE));
#   endif
    
#   undef EPKTlen
#   undef EPKTptr
#   undef EPKT
#   undef IPKTptr
#   undef IPKTlen
#   undef IPKT
#   undef MPKTptr
#   undef MPKT
#   undef LOGTAG
}

/* ----------------------------------------------------------------------- *
 * Public: ether to ip injector
 * ----------------------------------------------------------------------- */

/* This function reads an Ethernet packet from the LAN interface and
   forwards it to the TUN interface.

   If the system is ready, only registered MAC addresses are processed.

   When starting up, there is a learning phase that lastst some minutes
   and emulates the MAC adress registry process usually handled by the
   apr processor.
   
   This is needed upon SHAT reboot when the client has still the LAN
   interface cached while SHAT starts from scratch witf an empty MAC
   registry.
*/

void ip2ether_injector (IP2E *ip2e) {
    IN_DEV  *idev = ip2e->idev ;
    TUN_DEV *tdev = ip2e->tdev ;
    
    /* ether frame comes first */
#   define EPKTlen (ether_len)
#   define EPKTmax (idev->mtu + sizeof (struct ethhdr))
#   define EPKTptr (((struct ethhdr*)(idev->iobuf+2)))
#   define EPKT(x) ((EPKTptr)->x)

    /* Followed by the ip frame, aligned on a 64 bit boundary relative to the
       start of the io buffer. We use the fact that an ethernet frame looks
       like (dest-addr[6], src-addr[6], type[2]). */
#   define IPKTlen (ether_len - sizeof (struct ethhdr))
#   define IPKTmax (idev->mtu)
#   define IPKTptr (((struct ip*)(((struct ethhdr*)(idev->iobuf+2))+1)))
#   define IPKT(x) ((IPKTptr)->x)

    /* tun device packet header */
#   define QPKTptr ((uint32_t*)(((char*)IPKTptr) - TUN_HEADER_LENGTH))
#   define QPKTlen (IPKTlen + 4)

    /* sub-protocols: udp/tcp */
#   define UPKTptr ((struct udphdr*)(IPKTptr + 1))
#   define UPKT(x) ((UPKTptr)->x)
#   define TPKT(x) (((struct tcphdr*)(IPKTptr + 1))->x)

    /* dhcp */
#   define DPKTptr   ((VOID *)(UPKTptr + 1))
#   define DPKTyiadr ((struct in_addr    *)(((uint32_t*)DPKTptr) +  4))
#   define DPKTchadr ((struct ether_addr *)(((uint32_t*)DPKTptr) +  7))
#   define DPKToptr                        (((uint32_t*)DPKTptr) + 59)
#   define DPKTmin   (sizeof (struct ethhdr) +  \
                      sizeof (struct ip)     +  \
                      sizeof (struct udphdr) +  \
                      60 * sizeof (uint32_t) + 3)
    
#   define LOGTAG(s)  "injector: " s

    struct sockaddr_ll sll;
    int ether_len;
    MAC_REC *mac_rec ;
#   ifdef ALLOW_WINS
    int wins_seen = 0 ;
#   endif

    socklen_t sll_len = sizeof (sll);

    /* get an ethernet packet from the interface */
    if ((EPKTlen = recvfrom (idev->eth_fd,
                             EPKTptr,
                             EPKTmax,
                             MSG_TRUNC,
                             (struct sockaddr*)&sll,
                             &sll_len)) < 0) {
        if (errno == EAGAIN)
            goto just_polling;
        else
            goto eth_read_error;
    }
    
    if (IPKTlen < sizeof (struct ip))
        goto drop_eth_small_frame;
    if (EPKTlen > EPKTmax)
        goto drop_eth_large_frame;

    switch (IPKT (ip_p)) {
    case IPPROTO_UDP:
        switch (UPKT (dest)) {
#       if defined (DROP_NETBIOS) || defined (ALLOW_WINS)
        case H16TON (137):
        case H16TON (138):
#       endif
#       ifdef ALLOW_WINS
            wins_seen = 1 ;
            break ;
#       endif
        case H16TON (68):
            /* dhcp has its own packet device handler, we are only
               interested in outgoing packets, anyway */
            if (UPKT (source) == H16TON (67) &&
                ip2e->dhcp &&
                EPKTlen >= DPKTmin) {
                if (memcmp (EPKT (h_source), &idev->mac, ETH_ALEN)    == 0 &&
                    memcmp (DPKTptr, dhrply_hdr, sizeof (dhrply_hdr)) == 0 &&
                    memcmp (DPKToptr, dhcookie,   sizeof (dhcookie))  == 0) {
                    /* Register that packet so we can distinguish gratious
                       arp requests from network scan arp requests. */
                    if (register_set (ip2e->dhcp, DPKTyiadr, DPKTchadr))
                        goto register_dhcp_frame ;
                }
            }
            /* fall through */
        case H16TON (67):
            goto drop_eth_frame ;      /* only processing tcp/udp/icmp */
        }
        break ;
    case IPPROTO_TCP:
    case IPPROTO_ICMP:
#       ifdef DROP_NETBIOS
        switch (TPKT (dest)) {
        case H16TON (137):
        case H16TON (138):
            goto drop_eth_frame ;  /* who wants this ? */
        }
#       endif
        break ;
    default:
        goto drop_eth_frame ;      /* only processing tcp/udp/icmp */
    }

    /* ignore outgoing packets */
    if (memcmp (EPKT (h_source), &idev->mac, ETH_ALEN) == 0)
        goto drop_eth_frame ;

    /* Not processing broadcasts (in general). */
    if (memcmp (EPKT (h_dest), ffff, ETH_ALEN) == 0) {
#       ifdef ALLOW_WINS
        if (wins_seen)
            wins_seen = -1 ;
        else
#       endif
            goto drop_eth_frame;
    }

    /*  We do not process frames from an excluded IP source address. This
        range contains also the LAN interface addresses. */
    if (known_address (ip2e->ignore, &IPKT (ip_src))) 
        goto drop_eth_frame;

#   ifdef _IP2ETHER_DEBUG
    if (verbosity & IP2E_INBOUND_LOG)
        logger (LOG_INFO, LOGTAG ("<< %s: %s"),
                ethaddr_pp ((struct ether_addr*)&sll.sll_addr),
                ipkt_pp (IPKTptr, verbosity & IP2E_INBOUND_MORE));
#   endif


    if (
#       ifdef ALLOW_WINS
        wins_seen == -1 ||
#       endif
        time (0) < ip2e->end_start_up) {
        
        /* Register source mac/ip address pair while in learning mode.
           We expect that the arp pool is present (otherwise we should
           have not been called, at all). */
        mac_rec = make_mac_address (ip2e->pool,
                                    &IPKT (ip_src), /* real source ip */
                                    (struct ether_addr*)EPKT (h_source));
        
        if (mac_rec == 0) /* no more pool addresses */
            goto error_eth_frame_register;
    }
    else {
        /* normal operational mode, only registered addresses count */
        if ((mac_rec = find_mac_address
             (ip2e->pool, (struct ether_addr*)EPKT (h_source))) == 0)
            goto drop_eth_frame_unregistered;
        /* check whether the ip or mac address is spoofed */
        if (memcmp (&mac_rec->real_ip,
                    &IPKT (ip_src),
                    sizeof (struct in_addr)) != 0)
            goto drop_eth_frame_spoofed;
    }



    {   /* assign a source ip and calculate/update the checksums */
        uint32_t cks_delta;
        /* set source address to mapped value */
        cks_delta  = ((uint16_t*)&IPKT (ip_src)) [0];
        cks_delta += ((uint16_t*)&IPKT (ip_src)) [1];
        memcpy (&IPKT (ip_src),
                &mac_rec->ip4_rec->pool_ip,
                sizeof (struct in_addr));
        cks_delta -= ((uint16_t*)&IPKT (ip_src)) [0];
        cks_delta -= ((uint16_t*)&IPKT (ip_src)) [1];

        /* forward local destination data to the tun interface */
        if (
#           ifdef ALLOW_WINS
            wins_seen == -1 ||
#           endif
            memcmp (&IPKT (ip_dst),
                    &idev->ipa,
                    sizeof (struct in_addr)) == 0) {
            /* use the tun interface address instead  */
            cks_delta  += ((uint16_t*)&IPKT (ip_dst)) [0];
            cks_delta  += ((uint16_t*)&IPKT (ip_dst)) [1];
            memcpy (&IPKT (ip_dst),
                    &tdev->ipa,
                    sizeof (struct in_addr)) ;
            cks_delta  -= ((uint16_t*)&IPKT (ip_dst)) [0];
            cks_delta  -= ((uint16_t*)&IPKT (ip_dst)) [1];
        }

        switch (IPKT (ip_p)) {
            uint32_t ck_sum;
        case IPPROTO_UDP:
            /* correct the udp header checksum */
            ck_sum = UPKT (check) + cks_delta ;
            UPKT (check) = ck_sum + (ck_sum >> 16) ;
            break;
        case IPPROTO_TCP:
            /* correct the tcp header checksum */
            ck_sum = TPKT (check) + cks_delta ;
            TPKT (check) = ck_sum + (ck_sum >> 16) ;
            break;
        }
        /* recalculate ip checksum */
        IPKT (ip_sum) = ip_cksum (IPKTptr) ;
    }
    

    /* prepend by the tun header */
    *QPKTptr = htonl (TUN_HEADER_SIGHL);
    if (write (tdev->tun_fd, QPKTptr, QPKTlen) < 0)
        goto error_ip_frame;

#   ifdef _IP2ETHER_DEBUG
    if (verbosity & IP2E_INBOUND_LOG)
        logger (LOG_INFO, LOGTAG (">> %s"),
                ipkt_pp (IPKTptr, verbosity & IP2E_INBOUND_MORE));
#   else
    if (verbosity & IP2E_INBOUND_LOG)
        logger (LOG_INFO, LOGTAG ("%s"),
                ipkt_pp (IPKTptr, verbosity & IP2E_INBOUND_MORE));
#   endif
    return;

    /*NOTREACHED*/

 register_dhcp_frame:
    if (verbosity & IP2E_DHCP_REGISTER)
        logger (LOG_INFO, LOGTAG ("register %s"),
                ethpkt_pp (EPKTptr, 1));
    return;

 drop_eth_frame:
    if (verbosity & IP2E_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping %s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return;

 drop_eth_frame_unregistered:
    if (verbosity & IP2E_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping unregistered %s"), 
                 ethpkt_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return;

 drop_eth_frame_spoofed:
    if (verbosity & IP2E_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping spoofed %s"), 
                 ethpkt_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return;

 drop_eth_large_frame:
    /* oops: somebody changed the mtu? */
    if (verbosity & IP2E_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping lage ether: size %d > %d: %s"),
                EPKTlen, EPKTmax,
                ethhdr_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return ;

 drop_eth_small_frame:
    if (verbosity & IP2E_INBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping small ip: size %d < %d: %s"),
                IPKTlen, sizeof (struct ip),
                ethhdr_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return; 

 error_eth_frame_register:
    logger (LOG_INFO, LOGTAG ("ERROR %s: cannot register"), 
            ethpkt_pp (EPKTptr, verbosity & IP2E_INBOUND_MORE));
    return;

 error_ip_frame:
    logger (LOG_INFO, LOGTAG ("ERROR %s: %s"),
            ipkt_pp (IPKTptr, verbosity & IP2E_INBOUND_MORE),
            strerror (errno));
    return;
  
 eth_read_error:
    logger (LOG_INFO, LOGTAG ("ether/ip read error: %s"), strerror (errno));
    return ;
  
 just_polling:
    if (verbosity & IP2E_JUST_POLLING)
        logger (LOG_INFO, LOGTAG ("polling for ether frames ..."));
    return ;

#   undef LOGTAG
#   undef EPKTlen
#   undef EPKTmax
#   undef EPKTptr
#   undef EPKT
#   undef IPKTlen
#   undef IPKTmax
#   undef IPKTptr
#   undef IPKT
#   undef QPKTlen
#   undef QPKTptr
#   undef UPKTptr
#   undef UPKT
#   undef DPKTptr
#   undef DPKTciadr
#   undef DPKTchadr
#   undef DPKToptr
#   undef DPKTmin
#   undef TPKT
}


/* ----------------------------------------------------------------------- *
 * Public: ip to ether extractor
 * ----------------------------------------------------------------------- */

/* This function reads an IP frame from the TUN
   device.

   If the destination address of a frame was previously registered, it
   is replaced by the real IP address of the client. Then the IP frame is
   enclosed in an Ethernet frame and sent to the client (using its MAC
   address).
*/

void ip2ether_xtractor (IP2E *ip2e) {
    IN_DEV  *idev = ip2e->idev ;
    TUN_DEV *tdev = ip2e->tdev ;

    /* ether frame comes first */
#   define EPKTlen (IPKTlen + sizeof (struct ethhdr))
#   define EPKTmax (tdev->mtu + sizeof (struct ethhdr))
#   define EPKTptr ((struct ethhdr*)(tdev->iobuf+2))
#   define EPKT(x) ((EPKTptr)->x)

    /* Followed by the ip frame, aligned on a 64 bit boundary relative to the
       start of the io buffer. We use the fact that an ethernet frame looks
       like (dest-addr[6], src-addr[6], type[2]). */
#   define IPKTlen (tun_len - TUN_HEADER_LENGTH)
#   define IPKTmax (tdev->mtu)
#   define IPKTptr ((struct ip*)((EPKTptr)+1))
#   define IPKT(x) ((IPKTptr)->x)

    /* tun device packet header, usualy 8 bytes */
#   define QPKTptr ((uint32_t*)(((char*)IPKTptr) - TUN_HEADER_LENGTH))
#   define QPKTlen                    (tun_len)
#   define QPKTmax                    (tdev->mtu + TUN_HEADER_LENGTH)
    
    /* sub-protocols: udp/tcp */
#   define UPKT(x) (((struct udphdr*)(IPKTptr + 1))->x)
#   define TPKT(x) (((struct tcphdr*)(IPKTptr + 1))->x)
  
#   define LOGTAG(s)                  "xtractor: " s

    struct sockaddr_ll sll;
    size_t tun_len ;
    MAC_REC *mac_rec ;
 
    /* get a packet from the tun device */
    if ((QPKTlen = read (tdev->tun_fd, QPKTptr, QPKTmax)) < 0) {
        if (errno == EAGAIN)
            goto just_polling;
        else
            goto ip_read_error;
    }

    if (IPKTlen < sizeof (struct ip))
        goto drop_ip_small_frame;
    if (IPKTlen < ntohs (IPKT (ip_len)))
        goto drop_ip_large_frame;
    
#   ifdef _IP2ETHER_DEBUG
    if (verbosity & IP2E_OUTBOUND_LOG)
        logger (LOG_INFO, LOGTAG ("<< %s"),
                ipkt_pp (IPKTptr, verbosity & IP2E_OUTBOUND_MORE));
#   endif


    /* check whether the ip protocol is acceptable */
    switch (IPKT (ip_p)) {
    case IPPROTO_UDP:
        switch (UPKT (dest)) {
#       if defined (DROP_NETBIOS) || defined (ALLOW_WINS)
        case H16TON (137):
        case H16TON (138):
#       endif
#       ifdef ALLOW_WINS
            break ;
#       endif
        case H16TON (67):
        case H16TON (68):
            goto drop_ip_frame ;  /* dhcp has its own raw device handler */
        }
        break ;
    case IPPROTO_TCP:
    case IPPROTO_ICMP:
#       ifdef DROP_NETBIOS
        switch (TPKT (dest)) {
        case H16TON (137):
        case H16TON (138):
            goto drop_ip_frame ;  /* who wnats this ? */
        }
#       endif
        break ;
    default:
        goto drop_ip_frame ;      /* only processing tcp, udp, icmp */
    }

    

    {   /* check for a known session */
        IP4_REC *ip4_rec = find_pool_address (ip2e->pool, &IPKT (ip_dst));
        if (ip4_rec == 0)
            goto drop_ip_frame_unregistered;
        mac_rec = ip4_rec->mac_rec ;
    }


    {   /* assign a target ip and calculate/update the checksums */
        uint32_t cks_delta ;
        /* set target ip address to client */
        cks_delta   = ((uint16_t*)&IPKT (ip_dst)) [0];
        cks_delta  += ((uint16_t*)&IPKT (ip_dst)) [1];
        memcpy (&IPKT (ip_dst), &mac_rec->real_ip, sizeof (struct in_addr)) ;
        cks_delta  -= ((uint16_t*)&IPKT (ip_dst)) [0];
        cks_delta  -= ((uint16_t*)&IPKT (ip_dst)) [1];

        /* check for administrative responses from the tun interface */
        if (memcmp (&IPKT (ip_src),
                    &tdev->ipa,
                    sizeof (struct in_addr)) == 0) {
            /* use the lan interface address instead  */
            cks_delta  += ((uint16_t*)&IPKT (ip_src)) [0];
            cks_delta  += ((uint16_t*)&IPKT (ip_src)) [1];
            memcpy (&IPKT (ip_src), &idev->ipa, sizeof (struct in_addr)) ;
            cks_delta  -= ((uint16_t*)&IPKT (ip_src)) [0];
            cks_delta  -= ((uint16_t*)&IPKT (ip_src)) [1];
        }

        switch (IPKT (ip_p)) {
            uint32_t ck_sum ;
        case IPPROTO_UDP:
            /* correct the udp header checksum */
            ck_sum = UPKT (check) + cks_delta ;
            UPKT (check) = ck_sum + (ck_sum >> 16) ;
            break;
        case IPPROTO_TCP:
            /* correct the tcp checksum */
            ck_sum = TPKT (check) + cks_delta ;
            TPKT (check) = ck_sum + (ck_sum >> 16) ;
            break; /* end tcp */
        }
        /* recalculate IP checksum */
        IPKT (ip_sum) = ip_cksum (IPKTptr) ;
    }

    
    /* provide link layer information */
    EPKT (h_proto) = htons (ETH_P_IP) ;
    memcpy (EPKT (h_source),       &idev->mac, ETH_ALEN);
    memcpy (EPKT (h_dest), &mac_rec->real_mac, ETH_ALEN);

    /* send ethernet packet to the client */
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = htons (ETH_P_IP);
    sll.sll_ifindex  = idev->ifindex ;
    memcpy (sll.sll_addr, &mac_rec->real_mac, sizeof (sll.sll_addr)) ;

    if (sendto (idev->eth_fd, EPKTptr, EPKTlen, 0, 
                (struct sockaddr*)&sll, sizeof (sll)) < 0)
        goto error_eth_frame;
    
#   ifdef _IP2ETHER_DEBUG
    if (verbosity & IP2E_OUTBOUND_LOG)
        logger (LOG_INFO, LOGTAG (">> %s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE));
#   else
    if (verbosity & IP2E_OUTBOUND_LOG)
        logger (LOG_INFO, LOGTAG ("%s"),
                ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE));
#   endif
    return ;

    /* NOTREACHED */

drop_ip_frame:
    if (verbosity & IP2E_OUTBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping: %s"),
                 ipkt_pp (IPKTptr, verbosity & IP2E_OUTBOUND_MORE));
    return;

drop_ip_frame_unregistered:
    if (verbosity & IP2E_OUTBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping unregisterd: %s"), 
                 ipkt_pp (IPKTptr, verbosity & IP2E_OUTBOUND_MORE));
    return;

drop_ip_small_frame:
    if (verbosity & IP2E_OUTBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping small ip: size %d < %d: %s"),
                 IPKTlen, sizeof (struct ip), ipaddr_pp (&IPKT (ip_dst)));
    return; 

drop_ip_large_frame:
    /* oops: somebody changed the mtu? */
    if (verbosity & IP2E_OUTBOUND_DROP)
        logger (LOG_INFO, LOGTAG ("dropping large ip: size %d > %d: %s"),
                ntohs (IPKT (ip_len)), IPKTlen,
                ipkt_pp (IPKTptr, verbosity & IP2E_OUTBOUND_MORE));
    return; 

error_eth_frame:
    logger (LOG_INFO, LOGTAG ("ERROR %s: %s"),
            ethpkt_pp (EPKTptr, verbosity & IP2E_OUTBOUND_MORE),
            strerror (errno));
    return;

ip_read_error:
    logger (LOG_INFO, LOGTAG ("ip read error: %s"), strerror (errno));
    return ;

just_polling:
    if (verbosity & IP2E_JUST_POLLING)
        logger (LOG_INFO, LOGTAG ("polling for ip frames ..."));

#   undef LOGTAG
#   undef EPKTlen
#   undef EPKTmax
#   undef EPKTptr
#   undef EPKT
#   undef IPKTlen
#   undef IPKTmax
#   undef IPKTptr
#   undef IPKT
#   undef QPKTptr
#   undef QPKTlen
#   undef QPKTmax
#   undef UPKT
#   undef TPKT
}

/* ------------------------------------------------------------------------ *
 * Constructor/Destructor
 * ------------------------------------------------------------------------ */

IP2E *ip2ether_init (TUN_DEV  *tdev,
                     IN_DEV   *idev,
                     LOOKUP   *pool,
                     RANGE  *ignore,
                     REGISTER *dhcp) {

    IP2E *i = XMALLOC (IP2E);
    
    i->idev   =   idev; /* interface descriptor */
    i->tdev   =   tdev; /* interface descriptor */
    i->pool   =   pool; /* pool to assign addresses from */
    i->ignore = ignore; /* ignore these ip addresses */
    i->dhcp   =   dhcp;

    /* register any frame while starting up */
    i->end_start_up = time (0) + IP2E_LEARNING_PHASE ;

    return i;
}

void     ip2ether_close   (IP2E *i) { if (i) free (i); }
IN_DEV  *ip2ether_device  (IP2E *i) { return i ? i->idev   : 0; }
TUN_DEV *ip2ether_tundev  (IP2E *i) { return i ? i->tdev   : 0; }
LOOKUP  *ip2ether_pool    (IP2E *i) { return i ? i->pool   : 0; }
RANGE   *ip2ether_exclude (IP2E *i) { return i ? i->ignore : 0; }

/* ------------------------------------------------------------------------ *
 * End
 * ------------------------------------------------------------------------ */
