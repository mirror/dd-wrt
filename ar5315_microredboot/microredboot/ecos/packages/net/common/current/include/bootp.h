//==========================================================================
//
//      include/bootp.h
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
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

#ifdef __ECOS

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#include <machine/types.h>
#else
#include "bptypes.h"	/* for int32, u_int32 */
#endif

#define BP_CHADDR_LEN	 16
#define BP_SNAME_LEN	 64
#define BP_FILE_LEN	128

/* std min packet size to transmit for DHCP relays to work */
#define BP_STD_TX_MINPKTSZ     (300)

#ifdef CYGPKG_NET_DHCP
// The standard requires only 312 bytes here
#define BP_VEND_LEN	(312 + 32)
#define BP_MINPKTSZ	576
#else
#define BP_VEND_LEN	 64
#define BP_MINPKTSZ	300	/* to check sizeof(struct bootp) */
#endif

#define BP_MAX_OPTION_LEN 256

struct bootp {
    unsigned char    bp_op;			/* packet opcode type */
    unsigned char    bp_htype;			/* hardware addr type */
    unsigned char    bp_hlen;			/* hardware addr length */
    unsigned char    bp_hops;			/* gateway hops */
#ifdef __ECOS
    u_int32_t        bp_xid;			/* transaction ID */
#else
    unsigned int     bp_xid;			/* transaction ID */
#endif
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
};

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


/*
 * "vendor" data permitted for CMU bootp clients.
 */

struct cmu_vend {
	char		v_magic[4];	/* magic number */
#ifdef __ECOS
	u_int32_t	v_flags;	/* flags/opcodes, etc. */
#else
        unsigned int32  v_flags;        /* flags/opcodes, etc. */
#endif
	struct in_addr 	v_smask;	/* Subnet mask */
	struct in_addr 	v_dgate;	/* Default gateway */
	struct in_addr	v_dns1, v_dns2; /* Domain name servers */
	struct in_addr	v_ins1, v_ins2; /* IEN-116 name servers */
	struct in_addr	v_ts1, v_ts2;	/* Time servers */
#ifdef __ECOS
	int32_t		v_unused[6];	/* currently unused */
#else
        int32           v_unused[6];	/* currently unused */
#endif
};


/* v_flags values */
#define VF_SMASK	1	/* Subnet mask field contains valid data */

#ifdef __ECOS
#ifdef CYGHWR_NET_DRIVER_ETH0
extern struct bootp eth0_bootp_data;
extern cyg_bool_t   eth0_up;
extern const char  *eth0_name;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
extern struct bootp eth1_bootp_data;
extern cyg_bool_t   eth1_up;
extern const char  *eth1_name;
#endif

// ------------------------------------------------------------------------
// Initialize your own bootp record however you like, as far as is needed
// to bring up an interface.
__externC void
build_bootp_record(struct bootp *bp,
                   const char *if_name,
                   const char *addrs_ip,
                   const char *addrs_netmask,
                   const char *addrs_broadcast,
                   const char *addrs_gateway,
                   const char *addrs_server);

// Do bootp to fill in the bootp record from the net (other interfaces must
// be down for this to work, because of the "half-up" state of the
// interface in use)
__externC cyg_bool_t do_bootp(const char *interface, struct bootp *res);

// Initialize an interface (which is down) according to a bootp structure
__externC cyg_bool_t init_net(const char *interface, struct bootp *res);
#ifdef CYGPKG_NET_INET6
__externC cyg_bool_t init_net_IPv6(const char *intf, struct bootp *bp, char *prefix);
#endif

// Dump contents to diag_printf
__externC void show_bootp(const char *interface, struct bootp *res);

// Interrogate a bootp record for a particular option
__externC cyg_bool_t get_bootp_option(struct bootp *bp, unsigned char tag, void *res, unsigned int * length);

// ------------------------------------------------------------------------
// This isn't exactly the right place for this since bootp is not involved
// BUT you will only be using this API if you are using bootp-style
// initialization of the other interfaces; it fits here in a documentation
// sense.
__externC cyg_bool_t init_loopback_interface(int lo);

// ------------------------------------------------------------------------
// Do all the above automatically according to the configuration.  Do not
// mix using this and making the above calls yourself.
// (this is also declared in the much simpler API in network.h)
__externC void init_all_network_interfaces(void);

#endif

#endif // _BOOTP_H_
