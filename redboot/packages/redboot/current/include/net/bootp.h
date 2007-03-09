//==========================================================================
//
//      net/bootp.h
//
//      Stand-alone BOOTP support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

/************************************************************************
          Copyright 1988, 1991 by Carnegie Mellon University

                          All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of Carnegie Mellon University not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
************************************************************************/

#ifndef _BOOTP_H_
#define _BOOTP_H_

/*
 * Bootstrap Protocol (BOOTP).  RFC951 and RFC1395.
 *
 * This file specifies the "implementation-independent" BOOTP protocol
 * information which is common to both client and server.
 *
 */

#define BP_CHADDR_LEN	 16
#define BP_SNAME_LEN	 64
#define BP_FILE_LEN	128
#define BP_VEND_LEN	312
#define BP_MINPKTSZ	300	/* to check sizeof(struct bootp) */
#define BP_MIN_VEND_SIZE 64     /* minimum actual vendor area */

typedef struct bootp {
    unsigned char    bp_op;			/* packet opcode type */
    unsigned char    bp_htype;			/* hardware addr type */
    unsigned char    bp_hlen;			/* hardware addr length */
    unsigned char    bp_hops;			/* gateway hops */
    unsigned int     bp_xid;			/* transaction ID */
    unsigned short   bp_secs;			/* seconds since boot began */
    unsigned short   bp_flags;			/* RFC1532 broadcast, etc. */
    struct in_addr   bp_ciaddr;			/* client IP address */
    struct in_addr   bp_yiaddr;			/* 'your' IP address */
    struct in_addr   bp_siaddr;			/* server IP address */
    struct in_addr   bp_giaddr;			/* gateway IP address */
    unsigned char    bp_chaddr[BP_CHADDR_LEN];	/* client hardware address */
    char	     bp_sname[BP_SNAME_LEN];	/* server host name */
    char	     bp_file[BP_FILE_LEN];	/* boot file name */
    unsigned char    bp_vend[BP_VEND_LEN];	/* vendor-specific area */
    /* note that bp_vend can be longer, extending to end of packet. */
} bootp_header_t;

/*
 * UDP port numbers, server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

#define BOOTREPLY		2
#define BOOTREQUEST		1

/*
 * Hardware types from Assigned Numbers RFC.
 */
#define HTYPE_ETHERNET		  1
#define HTYPE_EXP_ETHERNET	  2
#define HTYPE_AX25		  3
#define HTYPE_PRONET		  4
#define HTYPE_CHAOS		  5
#define HTYPE_IEEE802		  6
#define HTYPE_ARCNET		  7

/*
 * Vendor magic cookie (v_magic) for CMU
 */
#define VM_CMU		"CMU"

/*
 * Vendor magic cookie (v_magic) for RFC1048
 */
#define VM_RFC1048	{ 99, 130, 83, 99 }



/*
 * Tag values used to specify what information is being supplied in
 * the vendor (options) data area of the packet.
 */
/* RFC 1048 */
/* End of cookie */
#define TAG_END			((unsigned char) 255)
/* padding for alignment */
#define TAG_PAD			((unsigned char)   0)
/* Subnet mask */
#define TAG_SUBNET_MASK		((unsigned char)   1)
/* Time offset from UTC for this system */
#define TAG_TIME_OFFSET		((unsigned char)   2)
/* List of routers on this subnet */
#define TAG_GATEWAY		((unsigned char)   3)
/* List of rfc868 time servers available to client */
#define TAG_TIME_SERVER		((unsigned char)   4)
/* List of IEN 116 name servers */
#define TAG_NAME_SERVER		((unsigned char)   5)
/* List of DNS name servers */
#define TAG_DOMAIN_SERVER	((unsigned char)   6)
/* List of MIT-LCS UDL log servers */
#define TAG_LOG_SERVER		((unsigned char)   7)
/* List of rfc865 cookie servers */
#define TAG_COOKIE_SERVER	((unsigned char)   8)
/* List of rfc1179 printer servers (in order to try) */
#define TAG_LPR_SERVER		((unsigned char)   9)
/* List of Imagen Impress servers (in prefered order) */
#define TAG_IMPRESS_SERVER	((unsigned char)  10)
/* List of rfc887 Resourse Location servers */
#define TAG_RLP_SERVER		((unsigned char)  11)
/* Hostname of client */
#define TAG_HOST_NAME		((unsigned char)  12)
/* boot file size */
#define TAG_BOOT_SIZE		((unsigned char)  13)
/* RFC 1395 */
/* path to dump to in case of crash */
#define TAG_DUMP_FILE		((unsigned char)  14)
/* domain name for use with the DNS */
#define TAG_DOMAIN_NAME		((unsigned char)  15)
/* IP address of the swap server for this machine */
#define TAG_SWAP_SERVER		((unsigned char)  16)
/* The path name to the root filesystem for this machine */
#define TAG_ROOT_PATH		((unsigned char)  17)
/* RFC 1497 */
/* filename to tftp with more options in it */
#define TAG_EXTEN_FILE		((unsigned char)  18)
/* RFC 1533 */
/* The following are in rfc1533 and may be used by BOOTP/DHCP */
/* IP forwarding enable/disable */
#define TAG_IP_FORWARD          ((unsigned char)  19)
/* Non-Local source routing enable/disable */
#define TAG_IP_NLSR             ((unsigned char)  20)
/* List of pairs of addresses/masks to allow non-local source routing to */
#define TAG_IP_POLICY_FILTER    ((unsigned char)  21)
/* Maximum size of datagrams client should be prepared to reassemble */
#define TAG_IP_MAX_DRS          ((unsigned char)  22)
/* Default IP TTL */
#define TAG_IP_TTL              ((unsigned char)  23)
/* Timeout in seconds to age path MTU values found with rfc1191 */
#define TAG_IP_MTU_AGE          ((unsigned char)  24)
/* Table of MTU sizes to use when doing rfc1191 MTU discovery */
#define TAG_IP_MTU_PLAT         ((unsigned char)  25)
/* MTU to use on this interface */
#define TAG_IP_MTU              ((unsigned char)  26)
/* All subnets are local option */
#define TAG_IP_SNARL            ((unsigned char)  27)
/* broadcast address */
#define TAG_IP_BROADCAST        ((unsigned char)  28)
/* perform subnet mask discovery using ICMP */
#define TAG_IP_SMASKDISC        ((unsigned char)  29)
/* act as a subnet mask server using ICMP */
#define TAG_IP_SMASKSUPP        ((unsigned char)  30)
/* perform rfc1256 router discovery */
#define TAG_IP_ROUTERDISC       ((unsigned char)  31)
/* address to send router solicitation requests */
#define TAG_IP_ROUTER_SOL_ADDR  ((unsigned char)  32)
/* list of static routes to addresses (addr, router) pairs */
#define TAG_IP_STATIC_ROUTES    ((unsigned char)  33)
/* use trailers (rfc893) when using ARP */
#define TAG_IP_TRAILER_ENC      ((unsigned char)  34)
/* timeout in seconds for ARP cache entries */
#define TAG_ARP_TIMEOUT         ((unsigned char)  35)
/* use either Ethernet version 2 (rfc894) or IEEE 802.3 (rfc1042) */
#define TAG_ETHER_IEEE          ((unsigned char)  36)
/* default TCP TTL when sending TCP segments */
#define TAG_IP_TCP_TTL          ((unsigned char)  37)
/* time for client to wait before sending a keepalive on a TCP connection */
#define TAG_IP_TCP_KA_INT       ((unsigned char)  38)
/* don't send keepalive with an octet of garbage for compatability */
#define TAG_IP_TCP_KA_GARBAGE   ((unsigned char)  39)
/* NIS domainname */
#define TAG_NIS_DOMAIN		((unsigned char)  40)
/* list of NIS servers */
#define TAG_NIS_SERVER		((unsigned char)  41)
/* list of NTP servers */
#define TAG_NTP_SERVER		((unsigned char)  42)
/* and stuff vendors may want to add */
#define TAG_VEND_SPECIFIC       ((unsigned char)  43)
/* NetBios over TCP/IP name server */
#define TAG_NBNS_SERVER         ((unsigned char)  44)
/* NetBios over TCP/IP NBDD servers (rfc1001/1002) */
#define TAG_NBDD_SERVER         ((unsigned char)  45)
/* NetBios over TCP/IP node type option for use with above */
#define TAG_NBOTCP_OTPION       ((unsigned char)  46)
/* NetBios over TCP/IP scopt option for use with above */
#define TAG_NB_SCOPE            ((unsigned char)  47)
/* list of X Window system font servers */
#define TAG_XFONT_SERVER        ((unsigned char)  48)
/* list of systems running X Display Manager (xdm) available to this client */
#define TAG_XDISPLAY_SERVER     ((unsigned char)  49)

/* While the following are only allowed for DHCP */
/* DHCP requested IP address */
#define TAG_DHCP_REQ_IP         ((unsigned char)  50)
/* DHCP time for lease of IP address */
#define TAG_DHCP_LEASE_TIME     ((unsigned char)  51)
/* DHCP options overload */
#define TAG_DHCP_OPTOVER        ((unsigned char)  52)
/* DHCP message type */
#define TAG_DHCP_MESS_TYPE      ((unsigned char)  53)
/* DHCP server identification */
#define TAG_DHCP_SERVER_ID      ((unsigned char)  54)
/* DHCP ordered list of requested parameters */
#define TAG_DHCP_PARM_REQ_LIST  ((unsigned char)  55)
/* DHCP reply message */
#define TAG_DHCP_TEXT_MESSAGE   ((unsigned char)  56)
/* DHCP maximum packet size willing to accept */
#define TAG_DHCP_MAX_MSGSZ      ((unsigned char)  57)
/* DHCP time 'til client needs to renew */
#define TAG_DHCP_RENEWAL_TIME   ((unsigned char)  58)
/* DHCP  time 'til client needs to rebind */
#define TAG_DHCP_REBIND_TIME    ((unsigned char)  59)
/* DHCP class identifier */
#define TAG_DHCP_CLASSID        ((unsigned char)  60)
/* DHCP client unique identifier */
#define TAG_DHCP_CLIENTID       ((unsigned char)  61)

/* XXX - Add new tags here */

/* DHCP Message Types */
#define DHCP_MESS_TYPE_DISCOVER  ((unsigned char) 1)
#define DHCP_MESS_TYPE_OFFER     ((unsigned char) 2)
#define DHCP_MESS_TYPE_REQUEST   ((unsigned char) 3)
#define DHCP_MESS_TYPE_DECLINE   ((unsigned char) 4)
#define DHCP_MESS_TYPE_ACK       ((unsigned char) 5)
#define DHCP_MESS_TYPE_NAK       ((unsigned char) 6)
#define DHCP_MESS_TYPE_RELEASE   ((unsigned char) 7)

/*
 * "vendor" data permitted for CMU bootp clients.
 */

struct cmu_vend {
	char		v_magic[4];	/* magic number */
        unsigned int    v_flags;        /* flags/opcodes, etc. */
	struct in_addr 	v_smask;	/* Subnet mask */
	struct in_addr 	v_dgate;	/* Default gateway */
	struct in_addr	v_dns1, v_dns2; /* Domain name servers */
	struct in_addr	v_ins1, v_ins2; /* IEN-116 name servers */
	struct in_addr	v_ts1, v_ts2;	/* Time servers */
        int             v_unused[6];	/* currently unused */
};


/* v_flags values */
#define VF_SMASK	1	/* Subnet mask field contains valid data */

#define IPPORT_BOOTPS           67
#define IPPORT_BOOTPC           68

#endif // _BOOTP_H_
