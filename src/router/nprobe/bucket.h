/*
 *  Copyright (C) 2002-04 Luca Deri <deri@ntop.org>
 *
 *  			  http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* ********************************** */

#define ENABLE_MAGIC

/* ********************************** */

/*
 * fallbacks for essential typedefs
 */
#ifdef WIN32
#ifndef __GNUC__
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
#endif
typedef u_char  uint8_t;
typedef u_short uint16_t;
typedef u_int   uint32_t;
#endif /* WIN32 */

#if !defined(HAVE_U_INT64_T)
#if defined(WIN32) && defined(__GNUC__)
typedef unsigned long long u_int64_t; /* on mingw unsigned long is 32 bits */
#else 
#if defined(WIN32)
typedef _int64 u_int64_t;
#else
#if defined(HAVE_UINT64_T)
#define u_int64_t uint64_t
#else
#error "Sorry, I'm unable to define u_int64_t on your platform"
#endif
#endif
#endif
#endif

#if !defined(HAVE_U_INT32_T)
typedef unsigned int u_int32_t;
#endif

#if !defined(HAVE_U_INT16_T)
typedef unsigned short u_int16_t;
#endif

#if !defined(HAVE_U_INT8_T)
typedef unsigned char u_int8_t;
#endif

#if !defined(HAVE_INT32_T)
typedef int int32_t;
#endif

#if !defined(HAVE_INT16_T)
typedef short int16_t;
#endif

#if !defined(HAVE_INT8_T)
typedef char int8_t;
#endif

/* ********************************** */

#define NPROBE_FD_SET(n, p)   (*(p) |= (1 << (n)))
#define NPROBE_FD_CLR(n, p)   (*(p) &= ~(1 << (n)))
#define NPROBE_FD_ISSET(n, p) (*(p) & (1 << (n)))
#define NPROBE_FD_ZERO(p)     (*(p) = 0)


#define FINGERPRINT_LEN          20
#define MAX_PAYLOAD_LEN          1400 /* bytes */

#define FLAG_NW_LATENCY_COMPUTED           1
#define FLAG_APPL_LATENCY_COMPUTED         2
#define FLAG_FRAGMENTED_PACKET_SRC2DST     3
#define FLAG_FRAGMENTED_PACKET_DST2SRC     4


#define nwLatencyComputed(a)          (NPROBE_FD_ISSET(FLAG_NW_LATENCY_COMPUTED,       &(a->flags)))
#define applLatencyComputed(a)        (NPROBE_FD_ISSET(FLAG_APPL_LATENCY_COMPUTED,     &(a->flags)))
#define fragmentedPacketSrc2Dst(a)    (NPROBE_FD_ISSET(FLAG_FRAGMENTED_PACKET_SRC2DST, &(a->flags)))
#define fragmentedPacketDst2Src(a)    (NPROBE_FD_ISSET(FLAG_FRAGMENTED_PACKET_DST2SRC, &(a->flags)))


#ifdef WIN32

#define _WS2TCPIP_H_ /* Avoid compilation problems */
#define IPV4_ONLY    /* On Win32 we support just IPv4 as transport */

/* IPv6 address */
/* Already defined in WS2tcpip.h */
struct win_in6_addr
  {
    union
      {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
      } in6_u;
/* #define s6_addr                 in6_u.u6_addr8 */
#define s6_addr16               in6_u.u6_addr16
#define s6_addr32               in6_u.u6_addr32
};

#define in6_addr win_in6_addr

struct ip6_hdr
  {
    union
      {
        struct ip6_hdrctl
          {
            uint32_t ip6_un1_flow;   /* 4 bits version, 8 bits TC,
                                        20 bits flow-ID */
            uint16_t ip6_un1_plen;   /* payload length */
            uint8_t  ip6_un1_nxt;    /* next header */
            uint8_t  ip6_un1_hlim;   /* hop limit */
          } ip6_un1;
        uint8_t ip6_un2_vfc;       /* 4 bits version, top 4 bits tclass */
      } ip6_ctlun;
    struct in6_addr ip6_src;      /* source address */
    struct in6_addr ip6_dst;      /* destination address */
};

/* Generic extension header.  */
struct ip6_ext
  {
    uint8_t  ip6e_nxt;		/* next header.  */
    uint8_t  ip6e_len;		/* length in units of 8 octets.  */
  };

#else /* WIN32 */

#ifndef s6_addr32
#ifdef linux
#define s6_addr32 in6_u.u6_addr32
#else
#if defined(sun) 
#define	s6_addr32	_S6_un._S6_u32
#else
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#endif
#endif
#endif /* WIN32*/

/* ********************************** */

#define MAX_NUM_MPLS_LABELS     10
#define MPLS_LABEL_LEN           3

/* ********************************** */

typedef struct ipAddress {
  u_int8_t ipVersion; /* Either 4 or 6 */
  
  union {
    struct in6_addr ipv6;
    u_int32_t ipv4;
  } ipType;
} IpAddress;

struct mpls_labels {
  u_short numMplsLabels;
  u_char mplsLabels[MAX_NUM_MPLS_LABELS][MPLS_LABEL_LEN];
};

struct pluginInfo; /* engine.h */

typedef struct pluginInformation {
  struct pluginInfo *pluginPtr;
  void *pluginData;
  struct pluginInformation *next;
} PluginInformation;

typedef struct hashBucket {
#ifdef ENABLE_MAGIC
  u_char magic;
#endif
  u_char bucket_expired;  /* Force bucket to expire */
  u_short proto;          /* protocol (e.g. UDP/TCP..) */
  u_char  srcMacAddress[6];
  IpAddress src;
  u_short sport;
  IpAddress dst;
  u_char  dstMacAddress[6];
  u_short dport;
  u_char src2dstTos, dst2srcTos;
  u_short vlanId;
  unsigned short src2dstTcpFlags, dst2srcTcpFlags;
  u_char src2dstFingerprint[FINGERPRINT_LEN], dst2srcFingerprint[FINGERPRINT_LEN];
  struct mpls_labels *mplsInfo;

  /* **************** */

  u_long bytesSent, pktSent;
  struct timeval firstSeenSent, lastSeenSent;
  u_long bytesRcvd, pktRcvd;
  struct timeval firstSeenRcvd, lastSeenRcvd;
  struct hashBucket *next;
  u_char src2dstPayloadLen;   /* # of bytes stored on the payload */
  unsigned char *src2dstPayload;
  u_char dst2srcPayloadLen;   /* # of bytes stored on the payload */
  unsigned char *dst2srcPayload;
  u_int32_t flags;               /* bitmask (internal) */
  struct timeval nwLatency;      /* network Latency (3-way handshake) */
  struct timeval src2dstApplLatency, dst2srcApplLatency; /* Application Latency */
  u_int32_t src2dstIcmpFlags, dst2srcIcmpFlags;  /* ICMP bitmask */
  u_int16_t src2dstIcmpType, dst2srcIcmpType;    /* ICMP type */
  PluginInformation *plugin;
} HashBucket;

