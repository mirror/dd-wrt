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
 * $Id: util.h,v 1.31 2005/04/30 12:00:21 jordan Exp $
 */

#ifndef __SHAT_UTIL_H
#define __SHAT_UTIL_H

#include <time.h>
#include <endian.h>
#include <syslog.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>



#include <netinet/ether.h>
#include <netinet/if_ether.h>


/* device.c */
#define DEFAULT_INBOUND      "eth1"
#define DEFAULT_TUNBOUND    "shat0"

#define DEV_NET_TIMEOUT    30 /* wait for the device to come up */
#define DEV_NET_TUN        "/dev/net/tun"

#define PROC_RPFITLER_FMT  "/proc/sys/net/ipv4/conf/%s/rp_filter"
#define PROC_PROXYARP_FMT  "/proc/sys/net/ipv4/conf/%s/proxy_arp"

#define TUN_DEFAULT_IP    "2.2.2.2"
#define TUN_HEADER_LENGTH        4
#define TUN_HEADER_SIGHL     0x800 /* tun signature, long host notation */

#define DEVICE_ARPADD_LOG   0x0001 /* log selector flag */


/* ip2ether.c */
#define IP2E_LEARNING_PHASE    300

#define IP2E_INBOUND_LOG    0x00001 /* log selector flag */
#define IP2E_OUTBOUND_LOG   0x00002 /* log selector flag */
#define IP2E_INBOUND_DROP   0x00004 /* log selector flag */
#define IP2E_OUTBOUND_DROP  0x00008 /* log selector flag */
#define IP2E_INBOUND_MORE   0x00010 /* log selector flag */
#define IP2E_OUTBOUND_MORE  0x00020 /* log selector flag */
#define IP2E_JUST_POLLING   0x00040 /* log selector flag */
#define IP2E_DHCP_REGISTER  0x00080 /* log selector flag */


/* arp.c */
#define ARP_REQUEST_LOG     0x00100 /* log selector flag */
#define ARP_REPLY_LOG       0x00200 /* log selector flag */
#define ARP_INBOUND_DROP    0x00400 /* log selector flag */
#define ARP_INBOUND_MORE    0x00800 /* log selector flag */
#define ARP_INBOUND_POLL    0x01000 /* log selector flag */

#define ARP_BATCH_DEFAULT_LENGTH 300 /* default length of reply queue */
#define ARP_SPOOF_DEFAULT_MAC "ac:de:48:xx:xx:xx" /* private mac header */
#define ARP_SPOOF_REPLIES          7 /* how many spoof replies to send */
#define ARP_SPOOF_REP_INTV        10 /* min secs betw consec frames */


/* shatd.c (main) */
#define LOG_IDLE_LOOPING    0x02000 /* log selector flag */
#define IDLE_TIMEOUT             30 /* secs timeout for idle loop */


/* cleanup.c */
#define CLEANUP_NOISY       0x04000 /* log selector flag */


/* lookup.c */
#define DEFAULT_REC_LEASE       400 /* timeout since last visit */


/* ctrl.c */
#define CTRL_REQUEST_LOG    0x08000 /* log selector flag */
#define CTRL_REPLY_LOG      0x10000 /* log selector flag */
#define CTRL_REQUEST_PATH   "/var/run/shat/server"
#define CTRL_IOBUF_SIZE        2000


/* util.c */
extern unsigned integer (unsigned *l, const char *in) ;
extern void logger (int level, const char *format, ...);
extern unsigned syslog_init (char *fac, int timestamp);

/* golbal variables */
extern char     *progname ;       /* program name */
extern unsigned verbosity ;       /* verbosity bits */

/* dummy VOID typdef forces explicit type cast */
typedef struct _void {void *any;} VOID;

/* ipv4 address in host/register byte order */
typedef uint32_t ip4addr_t ;

/* ether address in host/register byte order */
typedef uint64_t hw6addr_t ;

/* ip v4 address length, just to be used similar to ETH_ALEN */
#define IP4_ALEN 4

/* unaligned mac address pointer to hw4addr_t */
#define mp2hw6addr(mac) ((((((((((((hw6addr_t)(((uint8_t*)(mac)) [0]))\
		                   << 8) + (hw6addr_t)(((uint8_t*)(mac)) [1]))\
		                   << 8) + (hw6addr_t)(((uint8_t*)(mac)) [2]))\
		                   << 8) + (hw6addr_t)(((uint8_t*)(mac)) [3]))\
		                   << 8) + (hw6addr_t)(((uint8_t*)(mac)) [4]))\
		                   << 8) + (hw6addr_t)(((uint8_t*)(mac)) [5]))

/* hw6addr_t data value to unaligned address in memory */
#define hw6addr2ip(ptr,mac) (((uint8_t*)(ptr)) [0] = ((mac) >> 40),\
			                 ((uint8_t*)(ptr)) [1] = ((mac) >> 32),\
			                 ((uint8_t*)(ptr)) [2] = ((mac) >> 24),\
			                 ((uint8_t*)(ptr)) [3] = ((mac) >> 16),\
			                 ((uint8_t*)(ptr)) [4] = ((mac) >>  8),\
			                 ((uint8_t*)(ptr)) [5] = ((mac)      ))

/* unaligned ip address pointer to ip4addr_t */
#define ip2ip4addr(ip)      ((((((((ip4addr_t)(((uint8_t*)(ip)) [0]))\
	                   << 8) + (ip4addr_t)(((uint8_t*)(ip)) [1]))\
	                   << 8) + (ip4addr_t)(((uint8_t*)(ip)) [2]))\
	                   << 8) + (ip4addr_t)(((uint8_t*)(ip)) [3]))

/* ip4addr_t data value to unaligned address in memory */
#define ip4addr2ip(ptr,ip)  (((uint8_t*)(ptr)) [0] = ((ip) >> 24),\
			                 ((uint8_t*)(ptr)) [1] = ((ip) >> 16),\
			                 ((uint8_t*)(ptr)) [2] = ((ip) >>  8),\
			                 ((uint8_t*)(ptr)) [3] = ((ip)      ))

#if    __BYTE_ORDER == __LITTLE_ENDIAN
# define N16TOH(s) ((((s)>>8)&0xff) | (((s)<<8)&0xff00))
#elif  __BYTE_ORDER == __BIG_ENDIAN
# define N16TOH(s) (s)
#else
# error unknown byte order LITTLE or BIG ENDIAN 
#endif

#define H16TON(s) N16TOH(s)

#define LDX16B(p) \
        ((u_int16_t)*((const u_int8_t*)&(p) + 0) << 8 |\
         (u_int16_t)*((const u_int8_t*)&(p) + 1)) 

#define STX16B(p,val) \
        (*((u_int8_t*)&(p) + 0) = (u_int16_t)(val) >> 8,\
         *((u_int8_t*)&(p) + 1) = (u_int16_t)(val)     )

#define xstrlen(s) ((s) == 0 ? 0 : strlen (s))
#define xstrdup(s) strcpy ((char*)xmalloc (xstrlen (s) + 1), (s))
#define xmemdup(p) memcpy        (xmalloc (sizeof (*p)), p, sizeof (*p)) ;

/* secure malloc, zero-inizializes */
extern VOID *xmalloc (size_t);

/* secure realloc, does not initialize memory */
extern VOID *xrealloc (void *s, size_t);

/* some checksum helpers */
extern uint16_t   ip_cksum (const struct ip*);
extern uint16_t  udp_cksum (const struct ip*, unsigned short max);
extern uint16_t  tcp_cksum (const struct ip*, unsigned short max);
extern uint16_t icmp_cksum (const struct ip*, unsigned short max);

/* pretty print an ip, arp  or an ethernet packet header */
extern char   *ipkt_pp (const struct        ip*, int verbose);
extern char  *arpkt_pp (const struct ether_arp*, int verbose);
extern char *ethpkt_pp (const struct    ethhdr*, int verbose);
extern char *ethhdr_pp (const struct    ethhdr*, int verbose);

/* pretty print the given time stamp */
extern char *stamp_pp (const struct timeval *); /* ptr arg maybe NULL */
extern char *time_pp (time_t t) ;

/* pretty print integer pointer values */
extern char *ulongp_pp  (const  long *);  /* ptr arg maybe NULL */
extern char *ushortp_pp (const short *);  /* ptr arg maybe NULL */
extern char *xlongp_pp  (const  long *);  /* ptr arg maybe NULL */

/* pretty print an ipv4 address, keeps the last 4 return values 
   allocated on static memory */
extern char *ipaddr_pp (const struct in_addr *); /* ptr arg maybe NULL */
extern char *ip4addr_pp (ip4addr_t) ;

/* pretty print an ethernet address, keeps the last 4 return values 
   allocated on static memory */
extern char *ethaddr_pp (const struct ether_addr *);  /* ptr arg m'be NULL */
extern char *hw6addr_pp (hw6addr_t);

/* convenience macros */
#define XZERO(var)                 memset (&(var),  0, sizeof (var))
#define XREALLOC(p,type,n) ((type*)xrealloc ((p), (n)*sizeof (type)))
#define XMALLOC(   type)   ((type*)xmalloc(sizeof (type)))

// #define ETHER_PP(e) ether_pp ((const struct ether_addr*)(e))

/* typical netmask/network lookup */
#if _DEBUG
extern unsigned ip_and_eq (const struct in_addr *test,
                           const struct in_addr * and,
                           const struct in_addr *iseq);
#else
#      define ip_and_eq(t,a,i) (((t)->s_addr & (a)->s_addr) == (i)->s_addr)
#endif


#endif /* __SHAT_UTIL_H */
