/*
 *  $Id: libnet-ospf.h,v 1.1 2004/04/27 01:30:54 dyang Exp $
 *
 *  libnet-ospf.h - Network routine library headers header file
 *
 *  Copyright (c) 1999 Andrew Reiter <areiter@bindview.com>
 *  Bindview Development
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

#ifndef __LIBNET_OSPF_H
#define __LIBNET_OSPF_H

#include <sys/types.h>


/*
 *  Not all OSes define this in /etc/protocols.. ie, solaris...
 */
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF	    89
#endif
#define IPPROTO_OSPF_LSA    890         /* Made this up.  Hope it's unused */
#define OSPFVERSION         2
#define LIBNET_MODX                4102        /* Used in LSA checksum */


/*
 *  OSPF header lengths.
 */
#define LIBNET_OSPF_H       0x10    /* 16 bytes */
#define LIBNET_HELLO_H      0x18    /* 24 bytes */
#define LIBNET_DBD_H        0x8     /* 8 bytes */
#define LIBNET_LSR_H        0xc     /* 12 bytes */
#define LIBNET_LSU_H        0x4     /* 4 bytes */
#define LIBNET_LSA_H        0x14    /* 20 bytes */
#define LIBNET_AUTH_H       0x8     /* 8 bytes */
#define LIBNET_OSPF_CKSUM   0x10    /* 16 bytes */


/*
 *  Link State packet format lengths.
 */
#define LIBNET_LS_RTR_LEN   	0x10	        /* 16 bytes */
#define LIBNET_LS_NET_LEN	0x8	        /* 8 bytes */
#define LIBNET_LS_SUM_LEN	0xc             /* 12 bytes */	
#define LIBNET_LS_AS_EXT_LEN	0x10            /* 16 bytes */	


/*
 *  OSPFv2 Packet Header.
 */
struct libnet_ospf_hdr
{
    u_char ospf_v;                   /* version */
    u_char ospf_type;                /* type */
#define  LIBNET_OSPF_UMD       0     /* UMd monitoring packet */
#define  LIBNET_OSPF_HELLO     1     /* HELLO packet */
#define  LIBNET_OSPF_DBD       2     /* DataBase Description Packet */
#define  LIBNET_OSPF_LSR       3     /* Link State Request Packet */
#define  LIBNET_OSPF_LSU       4     /* Link State Update Packet */
#define  LIBNET_OSPF_LSA       5     /* Link State Acknowledgement Packet */
    u_short   ospf_len;              /* length */
    struct in_addr  ospf_rtr_id;     /* source router ID */ 
    struct in_addr  ospf_area_id;    /* roam ID */
    u_short ospf_cksum;              /* checksum */
    u_short ospf_auth_type;          /* authentication type */
#define LIBNET_OSPF_AUTH_NULL   0    /* Null password */
#define LIBNET_OSPF_AUTH_SIMPLE 1    /* Simple, plaintext, 8 char password */
#define LIBNET_OSPF_AUTH_MD5    2    /* MD5 */
};


/*
 *  OSPF authentication header.
 */
struct libnet_auth_hdr
{
    u_short ospf_auth_null;         /* NULL */
    u_char  ospf_auth_keyid;        /* Authentication Key ID */
    u_char  ospf_auth_len;          /* Auth data length */
    u_int   ospf_auth_seq;          /* Cryptographic sequence number */
};


/*
 *  Options used in multiple OSPF packets.
 *  More info can be found in section A.2 of RFC 2328.
 */
#define LIBNET_OPT_EBIT  0x02 /* Describes the way AS-external-LSAs are flooded */
#define LIBNET_OPT_MCBIT 0x04 /* Whether or not IP multicast dgrams are fwdd */
#define LIBNET_OPT_NPBIT 0x08 /* Describes handling of type-7 LSAs */
#define LIBNET_OPT_EABIT 0x10 /* rtr's willingness to send/recv EA-LSAs */
#define LIBNET_OPT_DCBIT 0x20 /* Describes handling of demand circuits */


/*
 *  OSPF HELLO Packet Header.
 */
struct libnet_ospf_hello_hdr
{
    struct in_addr hello_nmask; /* Netmask associated with the interface */
    u_short hello_intrvl;       /* Num of seconds between routers last packet */
    u_char hello_opts;          /* Options for HELLO packets (look above) */
    u_char hello_rtr_pri;       /* Router's priority (if 0, can't be backup) */
    u_int hello_dead_intvl;     /* # of secs a router is silent till deemed down */
    struct in_addr hello_des_rtr;   /* Designated router on the network */
    struct in_addr hello_bkup_rtr;  /* Backup router */
    struct in_addr hello_nbr;	    /* neighbor router, memcpy more as needed */
};


/*
 *  Database Description header.
 */
struct libnet_dbd_hdr
{
    u_short dbd_mtu_len;    /* Max length of IP dgram that this 'if' can use */
    u_char dbd_opts;        /* DBD packet options (look above) */
    u_char dbd_type;        /* Type of exchange occurring (look below) */
#define LIBNET_DBD_IBIT    0x01    /* Init Bit */
#define LIBNET_DBD_MBIT    0x02    /* Says more DBD packets are to come */
#define LIBNET_DBD_MSBIT   0x04  /* If 1, sender is the master in the exchnge */
    u_int  dbd_seq;         /* DBD sequence number */
};


/*
 *  Used for the LS type field in all LS* headers.
 */
#define LIBNET_LS_TYPE_RTR	1   /* Router-LSA */
#define LIBNET_LS_TYPE_NET	2   /* Network-LSA */
#define LIBNET_LS_TYPE_IP	3   /* Summary-LSA (IP Network) */
#define LIBNET_LS_TYPE_ASBR    4   /* Summary-LSA (ASBR) */
#define LIBNET_LS_TYPE_ASEXT	5   /* AS-External-LSA */


/*
 *  Link State Request header.
 */
struct libnet_lsr_hdr
{
    u_int lsr_type;             /* Type of LS being requested (see below) */
    u_int lsr_lsid;             /* Link State ID */
    struct in_addr lsr_adrtr;   /* advertising router (memcpy more as needed) */
};


/*
 *  Link State Update header.
 */
struct libnet_lsu_hdr
{
    u_int lsu_num;              /* Number of LSAs that will be broadcasted */
};


/*
 *  Link State Acknowledgement header.
 */
struct libnet_lsa_hdr
{
    u_short lsa_age;        /* Time in seconds since the LSA was originated */
    u_char lsa_opts;        /* Look above for OPTS_* */
    u_char lsa_type;        /* Look below for LS_TYPE_* */
    u_int lsa_id;           /* Link State ID */
    struct in_addr lsa_adv; /* Router ID of Advertising router */
    u_int lsa_seq;          /* LSA sequence number to detect old/bad ones */
    u_char lsa_cksum[2];    /* "Fletcher Checksum" of all fields minus age */
    u_short lsa_len;        /* Length in bytes including the 20 byte header */
};


/*
 *  Router LSA data format
 *
 *  Other stuff for TOS can be added for backward compatability, for this
 *  version, only OSPFv2 is being FULLY supported.
 */

struct libnet_rtr_lsa_hdr
{
    u_short rtr_flags;      /* Set to help describe packet */
#define LIBNET_RTR_FLAGS_W     0x0100  /* W bit */
#define LIBNET_RTR_FLAGS_E     0x0200  /* E bit */
#define LIBNET_RTR_FLAGS_B     0x0400  /* B bit */
    u_short rtr_num;        /* Number of links within that packet */
    u_int rtr_link_id;      /* Describes link_data (look below) */
#define LIBNET_LINK_ID_NBR_ID  1       /* Neighbors router ID, also can be 4 */
#define LIBNET_LINK_ID_IP_DES  2       /* IP address of designated router */
#define LIBNET_LINK_ID_SUB     3       /* IP subnet number */
    u_int rtr_link_data;    /* Depending on link_id, info is here */
    u_char rtr_type;        /* Description of router link */
#define LIBNET_RTR_TYPE_PTP    1   /* Point-To-Point */
#define LIBNET_RTR_TYPE_TRANS  2   /* Connection to a "transit network" */
#define LIBNET_RTR_TYPE_STUB   3   /* Connectin to a "stub network" */
#define RTR_TYPE_VRTL   4   /* Connects to a "virtual link" */
    u_char rtr_tos_num;     /* Number of different TOS metrics for this link */
    u_short rtr_metric;     /* The "cost" of using this link */
};


/*
 *  Network LSA data format.
 */
struct libnet_net_lsa_hdr
{
    struct in_addr net_nmask;   /* Netmask for that network */
    u_int  net_rtr_id;          /* ID of router attached to that network */
};


/*
 *  Summary LSA data format.
 */
struct libnet_sum_lsa_hdr
{
    struct in_addr sum_nmask;   /* Netmask of destination IP address */
    u_int  sum_metric;          /* Same as in rtr_lsa (&0xfff to use last 24bit) */
    u_int  sum_tos_metric;      /* first 8bits are TOS, 24bits are TOS Metric */
};


/*
 *  AS External LSA data format.
 *  & 0xfff logic operator for as_metric to get last 24bits.
 */

struct libnet_as_lsa_hdr
{
    struct in_addr as_nmask;    /* Netmask for advertised destination */
    u_int  as_metric;           /* May have to set E bit in first 8bits */
#define LIBNET_AS_E_BIT_ON 0x80000000  /* as_metric */
    struct in_addr as_fwd_addr; /* Forwarding address */
    u_int  as_rte_tag;          /* External route tag */
};


int
libnet_build_ospf(
    u_short,
    u_char,
    u_long,
    u_long,
    u_short,
    const char *,
    int,
    u_char *
    );     


int
libnet_build_ospf_hello(
    u_long,
    u_short,
    u_char,
    u_char,
    u_int,
    u_long,
    u_long,
    u_long,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_dbd(
    u_short,
    u_char,
    u_char,
    u_int,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsr(
    u_int,
    u_int,
    u_long,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsu(
    u_int,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsa(
    u_short,
    u_char,
    u_char,
    u_int,
    u_long,
    u_int,
    u_short,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsa_rtr(
    u_short,
    u_short,
    u_int,
    u_int,
    u_char,
    u_char,
    u_short,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsa_net(
    u_long,
    u_int,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsa_sum(
    u_long,
    u_int,
    u_int,
    const char *,
    int,
    u_char *
    );


int
libnet_build_ospf_lsa_as(
    u_long,
    u_int,
    u_long, 
    u_int,
    const char *,
    int,
    u_char *
    );


void
libnet_ospf_lsa_checksum(
    u_char *,
    int
    );


#endif /* __LIBNET_OSPF_H */
