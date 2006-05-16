/*
 *  $Id: libnet-headers.h,v 1.1 2004/04/27 01:30:54 dyang Exp $
 *
 *  libnet-headers.h - Network routine library headers header file
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
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

#ifndef __LIBNET_HEADERS_H
#define __LIBNET_HEADERS_H

/* 
 *  Standard (IPv4) header sizes in bytes.
 */

#define LIBNET_ARP_H           0x1c    /* ARP header:          28 bytes */
#define LIBNET_DNS_H           0xc     /* DNS header base:     12 bytes */
#define LIBNET_ETH_H           0xe     /* Etherner header:     14 bytes */
#define LIBNET_ICMP_H          0x4     /* ICMP header base:     4 bytes */
#define LIBNET_ICMP_ECHO_H     0x8     /* ICMP_ECHO header:     8 bytes */
#define LIBNET_ICMP_MASK_H     0xc     /* ICMP_MASK header:    12 bytes */
#define LIBNET_ICMP_UNREACH_H  0x8     /* ICMP_UNREACH header:  8 bytes */
#define LIBNET_ICMP_TIMXCEED_H 0x8     /* ICMP_TIMXCEED header: 8 bytes */
#define LIBNET_ICMP_REDIRECT_H 0x8     /* ICMP_REDIRECT header: 8 bytes */
#define LIBNET_ICMP_TS_H       0x14    /* ICMP_TIMESTAMP headr:20 bytes */
#define LIBNET_IGMP_H          0x8     /* IGMP header:          8 bytes */
#define LIBNET_IP_H            0x14    /* IP header:           20 bytes */
/* See libnet-ospf.h for OSPF related header sizes */
#define LIBNET_RIP_H           0x18    /* RIP header base:     24 bytes */
#define LIBNET_TCP_H           0x14    /* TCP header:          20 bytes */
#define LIBNET_UDP_H           0x8     /* UDP header:           8 bytes */

/*
 *  Concession to legacy naming scheme.
 */
#define ARH_H           LIBNET_ARP_H
#define DNS_H           LIBNET_DNS_H
#define ETH_H           LIBNET_ETH_H
#define ICMP_H          LIBNET_ICMP_H
#define ICMP_ECHO_H     LIBNET_ICMP_ECHO_H
#define ICMP_MASK_H     LIBNET_ICMP_MASK_H
#define ICMP_UNREACH_H  LIBNET_ICMP_UNREACH_H
#define ICMP_TIMXCEED_H LIBNET_ICMP_TIMXCEED_H
#define ICMP_REDIRECT_H LIBNET_ICMP_REDIRECT_H
#define ICMP_TS_H       LIBNET_ICMP_TS_H
#define IGMP_H          LIBNET_IGMP_H
#define IP_H            LIBNET_IP_H
#define RIP_H           LIBNET_RIP_H
#define TCP_H           LIBNET_TCP_H
#define UDP_H           LIBNET_UDP_H

/*
 *  IP packet header prototype.
 */
struct libnet_ip_hdr
{
#if (LIBNET_LIL_ENDIAN)
    u_char ip_hl:4,         /* header length */
            ip_v:4;         /* version */
#endif
#if (LIBNET_BIG_ENDIAN)
    u_char ip_v:4,          /* version */
            ip_hl:4;        /* header length */
#endif
    u_char ip_tos;          /* type of service */
    u_short ip_len;         /* total length */
    u_short ip_id;          /* identification */
    u_short ip_off;
#ifndef IP_RF
#define IP_RF 0x8000        /* reserved fragment flag */
#endif
#ifndef IP_DF
#define IP_DF 0x4000        /* dont fragment flag */
#endif
#ifndef IP_MF
#define IP_MF 0x2000        /* more fragments flag */
#endif 
#ifndef IP_OFFMASK
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
#endif
    u_char ip_ttl;          /* time to live */
    u_char ip_p;            /* protocol */
    u_short ip_sum;         /* checksum */
    struct in_addr ip_src, ip_dst; /* source and dest address */
};

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535
#endif

/*
 *  TCP packet header prototype.
 */
struct libnet_tcp_hdr
{
    u_short th_sport;       /* source port */
    u_short th_dport;       /* destination port */
    u_long th_seq;          /* sequence number */
    u_long th_ack;          /* acknowledgement number */
#if (LIBNET_LIL_ENDIAN)
    u_char th_x2:4,         /* (unused) */
           th_off:4;        /* data offset */
#endif
#if (LIBNET_BIG_ENDIAN)
    u_char th_off:4,        /* data offset */
           th_x2:4;         /* (unused) */
#endif
    u_char  th_flags;       /* control flags */
#ifndef TH_FIN
#define TH_FIN    0x01
#endif
#ifndef TH_SYN
#define TH_SYN    0x02
#endif
#ifndef TH_RST
#define TH_RST    0x04
#endif
#ifndef TH_PUSH
#define TH_PUSH   0x08
#endif
#ifndef TH_ACK
#define TH_ACK    0x10
#endif
#ifndef TH_URG
#define TH_URG    0x20
#endif
    u_short th_win;         /* window */
    u_short th_sum;         /* checksum */
    u_short th_urp;         /* urgent pointer */
};


/*
 *  UDP packet header prototype.
 */
struct libnet_udp_hdr
{
    u_short uh_sport;   /* soure port */
    u_short uh_dport;   /* destination port */
    u_short uh_ulen;    /* length */
    u_short uh_sum;     /* checksum */
};


/*
 *  ICMP packet header prototype.
 */
struct libnet_icmp_hdr
{
    u_char icmp_type;
/*
 *  ICMP types.
 */
#ifndef     ICMP_ECHOREPLY
#define     ICMP_ECHOREPLY                  0
#endif
#ifndef     ICMP_UNREACH
#define     ICMP_UNREACH                    3
#endif
#ifndef     ICMP_SOURCEQUENCH
#define     ICMP_SOURCEQUENCH               4
#endif
#ifndef     ICMP_REDIRECT
#define     ICMP_REDIRECT                   5
#endif
#ifndef     ICMP_ECHO
#define     ICMP_ECHO                       8
#endif
#ifndef     ICMP_ROUTERADVERT
#define     ICMP_ROUTERADVERT               9
#endif
#ifndef     ICMP_ROUTERSOLICIT
#define     ICMP_ROUTERSOLICIT              10
#endif
#ifndef     ICMP_TIMXCEED
#define     ICMP_TIMXCEED                   11
#endif
#ifndef     ICMP_PARAMPROB
#define     ICMP_PARAMPROB                  12
#endif
#ifndef     ICMP_TSTAMP
#define     ICMP_TSTAMP                     13
#endif
#ifndef     ICMP_TSTAMPREPLY
#define     ICMP_TSTAMPREPLY                14
#endif
#ifndef     ICMP_IREQ
#define     ICMP_IREQ                       15
#endif
#ifndef     ICMP_IREQREPLY
#define     ICMP_IREQREPLY                  16
#endif
#ifndef     ICMP_MASKREQ
#define     ICMP_MASKREQ                    17
#endif
#ifndef     ICMP_MASKREPLY
#define     ICMP_MASKREPLY                  18
#endif
    u_char icmp_code;
/*
 *  ICMP codes.
 */
#ifndef     ICMP_UNREACH_NET
#define     ICMP_UNREACH_NET                0
#endif
#ifndef     ICMP_UNREACH_HOST
#define     ICMP_UNREACH_HOST               1
#endif
#ifndef     ICMP_UNREACH_PROTOCOL
#define     ICMP_UNREACH_PROTOCOL           2
#endif
#ifndef     ICMP_UNREACH_PORT
#define     ICMP_UNREACH_PORT               3
#endif
#ifndef     ICMP_UNREACH_NEEDFRAG
#define     ICMP_UNREACH_NEEDFRAG           4
#endif
#ifndef     ICMP_UNREACH_SRCFAIL
#define     ICMP_UNREACH_SRCFAIL            5
#endif
#ifndef     ICMP_UNREACH_NET_UNKNOWN
#define     ICMP_UNREACH_NET_UNKNOWN        6
#endif
#ifndef     ICMP_UNREACH_HOST_UNKNOWN
#define     ICMP_UNREACH_HOST_UNKNOWN       7
#endif
#ifndef     ICMP_UNREACH_ISOLATED
#define     ICMP_UNREACH_ISOLATED           8
#endif
#ifndef     ICMP_UNREACH_NET_PROHIB
#define     ICMP_UNREACH_NET_PROHIB         9
#endif
#ifndef     ICMP_UNREACH_HOST_PROHIB
#define     ICMP_UNREACH_HOST_PROHIB        10
#endif
#ifndef     ICMP_UNREACH_TOSNET
#define     ICMP_UNREACH_TOSNET             11
#endif
#ifndef     ICMP_UNREACH_TOSHOST
#define     ICMP_UNREACH_TOSHOST            12
#endif
#ifndef     ICMP_UNREACH_FILTER_PROHIB
#define     ICMP_UNREACH_FILTER_PROHIB      13
#endif
#ifndef     ICMP_UNREACH_HOST_PRECEDENCE
#define     ICMP_UNREACH_HOST_PRECEDENCE    14
#endif
#ifndef     ICMP_UNREACH_PRECEDENCE_CUTOFF
#define     ICMP_UNREACH_PRECEDENCE_CUTOFF  15
#endif
#ifndef     ICMP_REDIRECT_NET
#define     ICMP_REDIRECT_NET               0
#endif
#ifndef     ICMP_REDIRECT_HOST
#define     ICMP_REDIRECT_HOST              1
#endif
#ifndef     ICMP_REDIRECT_TOSNET
#define     ICMP_REDIRECT_TOSNET            2
#endif
#ifndef     ICMP_REDIRECT_TOSHOST
#define     ICMP_REDIRECT_TOSHOST           3
#endif
#ifndef     ICMP_TIMXCEED_INTRANS
#define     ICMP_TIMXCEED_INTRANS           0
#endif
#ifndef     ICMP_TIMXCEED_REASS
#define     ICMP_TIMXCEED_REASS             1
#endif
#ifndef     ICMP_PARAMPROB_OPTABSENT
#define     ICMP_PARAMPROB_OPTABSENT        1
#endif

    u_short icmp_sum;

    union
    {
        struct
        {
            u_short id;
            u_short seq;
        }echo;

#undef icmp_id
#undef icmp_seq
#define icmp_id     hun.echo.id
#define icmp_seq    hun.echo.seq
 
        u_long gateway;
        struct
        {
            u_short pad;
            u_short mtu;
        }frag;
    }hun;
    union
    {
        struct
        {
            n_time its_otime;
            n_time its_rtime;
            n_time its_ttime;
        }ts;
        struct
        {
            struct ip idi_ip;
            /* options and then 64 bits of data */
        }ip;
        u_long mask;
        char data[1];

#undef icmp_mask
#define icmp_mask    dun.mask
#undef icmp_data
#define icmp_data    dun.data

#undef icmp_otime
#define icmp_otime   dun.ts.its_otime
#undef icmp_rtime
#define icmp_rtime   dun.ts.its_rtime
#undef icmp_ttime
#define icmp_ttime   dun.ts.its_ttime
    }dun;
};


/*
 *  IGMP header.
 */
struct libnet_igmp_hdr
{
    u_char igmp_type;
#ifndef IGMP_MEMBERSHIP_QUERY
#define IGMP_MEMBERSHIP_QUERY           0x11    /* membership query */
#endif
#ifndef IGMP_V1_MEMBERSHIP_REPORT
#define IGMP_V1_MEMBERSHIP_REPORT       0x12    /* Ver. 1 membership report */
#endif
#ifndef IGMP_V2_MEMBERSHIP_REPORT
#define IGMP_V2_MEMBERSHIP_REPORT       0x16    /* Ver. 2 membership report */
#endif
#ifndef IGMP_LEAVE_GROUP
#define IGMP_LEAVE_GROUP                0x17    /* Leave-group message */
#endif
    u_char igmp_code;
    u_short igmp_sum;
    struct in_addr igmp_group;
};


/* 
 *  Ethernet packet header prototype.  Too many O/S's define this differently.
 *  Easy enough to solve that and define it here.
 */
struct libnet_ethernet_hdr
{
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif
    u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination ethernet address */
    u_char  ether_shost[ETHER_ADDR_LEN];    /* source ethernet address */
    u_short ether_type;                     /* packet type ID */
};

#define ETHERTYPE_PUP           0x0200  /* PUP protocol */
#define ETHERTYPE_IP            0x0800  /* IP protocol */
#define ETHERTYPE_ARP           0x0806  /* Addr. resolution protocol */
#define ETHERTYPE_REVARP        0x8035  /* reverse Addr. resolution protocol */
#define ETHERTYPE_VLAN          0x8100  /* IEEE 802.1Q VLAN tagging */
#define ETHERTYPE_LOOPBACK      0x9000  /* used to test interfaces */

#if (!__GLIBC__)
struct ether_addr
{
    u_char  ether_addr_octet[6];
};
#endif

/* 
 *  ARP packet header prototype.  Too many O/S's define this differently.
 *  Easy enough to solve that and define it here.
 */
struct libnet_arp_hdr
{
    u_short ar_hrd;                         /* format of hardware address */
#define ARPHRD_ETHER     1                  /* ethernet hardware format */
    u_short ar_pro;                         /* format of protocol address */
    u_char  ar_hln;                         /* length of hardware address */
    u_char  ar_pln;                         /* length of protocol addres */
    u_short ar_op;                          /* operation type */
#define ARPOP_REQUEST    1                  /* req to resolve address */
#define ARPOP_REPLY      2                  /* resp to previous request */
#define ARPOP_REVREQUEST 3                  /* req protocol address given hardware */
#define ARPOP_REVREPLY   4                  /* resp giving protocol address */
#define ARPOP_INVREQUEST 8                  /* req to identify peer */
#define ARPOP_INVREPLY   9                  /* resp identifying peer */

    /*
     *  These should implementation defined but I've hardcoded eth/IP.
     */
    u_char ar_sha[6];                         /* sender hardware address */
    u_char ar_spa[4];                         /* sender protocol address */
    u_char ar_tha[6];                         /* target hardware address */
    u_char ar_tpa[4];                         /* target protocol address */
};


/*
 *  Base DNS header.
 */
struct libnet_dns_hdr
{
    u_short id;             /* DNS packet ID */
    u_short flags;          /* DNS flags */
    u_short num_q;          /* Number of questions */
    u_short num_answ_rr;    /* Number of answer resource records */
    u_short num_auth_rr;    /* Number of authority resource records */
    u_short num_addi_rr;    /* Number of additional resource records */
};


/*
 *  Base RIP (routing information protocol) header.
 */
struct libnet_rip_hdr
{
    u_char cmd;             /* RIP command */
#define RIPCMD_REQUEST      1   /* want info */
#define RIPCMD_RESPONSE     2   /* responding to request */
#define RIPCMD_TRACEON      3   /* turn tracing on */
#define RIPCMD_TRACEOFF     4   /* turn it off */
#define RIPCMD_POLL         5   /* like request, but anyone answers */
#define RIPCMD_POLLENTRY    6   /* like poll, but for entire entry */
#define RIPCMD_MAX          7
    u_char ver;             /* RIP version */
#define RIPVER_0            0
#define RIPVER_1            1
#define RIPVER_2            2
    u_short rd;             /* Zero (v1) or Routing Domain (v2) */
    u_short af;             /* Address family */
    u_short rt;             /* Zero (v1) or Route Tag (v2) */
    u_long addr;            /* IP address */
    u_long mask;            /* Zero (v1) or Subnet Mask (v2) */
    u_long next_hop;        /* Zero (v1) or Next hop IP address (v2) */
    u_long metric;          /* Metric */
};


#if 0
struct libnet_snmp_hdr
{
    /* ASN.1 BER support first */
};
#endif


/*
 *  TCP options structure.
 */
struct tcpoption
{
    u_char tcpopt_list[MAX_IPOPTLEN];
};


#if (__linux__)
/*
 *  Linux has a radically different IP options structure from BSD.
 */
struct ipoption
{
    struct  in_addr ipopt_dst;          /* first-hop dst if source routed */
    char ipopt_list[MAX_IPOPTLEN];      /* options proper */
};
#endif

#endif  /* __LIBNET_HEADERS_H */

/* EOF */
