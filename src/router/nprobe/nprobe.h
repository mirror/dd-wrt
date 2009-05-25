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


/* *************************** */

/* #define DEMO */

#define MAX_DEMO_FLOWS    2000
#ifdef DEMO
#define DEMO_MODE
#define MAKE_STATIC_PLUGINS
#endif

/* *************************** */


#include "config.h"

/* See http://www.redhat.com/magazine/009jul05/features/execshield/ */
#ifndef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 2
#endif

#if defined(linux) || defined(__linux__)
/*
 * This allows to hide the (minimal) differences between linux and BSD
 */
#include <features.h>
#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#endif /* linux || __linux__ */

#ifdef WIN32
#include <winsock2.h> /* winsock.h is included automatically */
#include <process.h>
#include "dirent.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>

/* Courtesy of Curt Sampson  <cjs@cynic.net> */
#ifdef __NetBSD__
#include <net/if_ether.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif

#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif

#include <netinet/in_systm.h>

#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#endif

#ifndef EMBEDDED
#include <sys/stat.h>
#endif

#include <pcap.h>

#include "bucket.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#ifdef HAVE_DL_H
#include <dl.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef MAKE_STATIC_PLUGINS
#undef HAVE_MYSQL
#else
#ifdef HAVE_MYSQL
#include <mysql.h>
#define MYSQL_OPT "--mysql="
#endif
#endif

#define TEMPLATE_LIST_LEN   32

typedef struct ether80211q {
  u_int16_t vlanId;
  u_int16_t protoType;
} Ether80211q;

#ifndef TH_FIN
#define	TH_FIN	0x01
#endif
#ifndef TH_SYN
#define	TH_SYN	0x02
#endif
#ifndef TH_RST
#define	TH_RST	0x04
#endif
#ifndef TH_PUSH
#define	TH_PUSH	0x08
#endif
#ifndef TH_ACK
#define	TH_ACK	0x10
#endif
#ifndef TH_URG
#define	TH_URG	0x20
#endif



#ifndef WIN32
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#else /* WIN32 */
#define pthread_t              HANDLE
#define pthread_mutex_t        HANDLE

/*
 * Ethernet address - 6 octets
 */
struct ether_addr {
  u_char ether_addr_octet[6];
};

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct ether_header {
  u_char	ether_dhost[6];
  u_char	ether_shost[6];
  u_short	ether_type;
};

#if !defined (__GNUC__)
typedef	u_int	tcp_seq;
#endif

/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	u_short	th_sport;		/* source port */
	u_short	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#else
	u_char	th_off:4,		/* data offset */
		th_x2:4;		/* (unused) */
#endif
	u_char	th_flags;
	u_short	th_win;			/* window */
	u_short	th_sum;			/* checksum */
	u_short	th_urp;			/* urgent pointer */
};

/* ********************************************* */

struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char	ip_hl:4,		/* header length */
		ip_v:4;			/* version */
#else
	u_char	ip_v:4,			/* version */
		ip_hl:4;		/* header length */
#endif
	u_char	ip_tos;			/* type of service */
	short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	short	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

/* ********************************************* */

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
struct udphdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	short	uh_ulen;		/* udp length */
	u_short	uh_sum;			/* udp checksum */
};

#define getopt getopt____

extern int gettimeofday(struct timeval *tv, struct timezone *tz);
extern char *strtok_r(char *s, const char *delim, char **save_ptr);
extern int nprobe_sleep(int secToSleep);

extern int pthread_create(pthread_t *threadId, void* notUsed, void *(*__start_routine) (void *), char* userParm);
extern void pthread_detach(pthread_t *threadId);
extern int pthread_mutex_init(pthread_mutex_t *mutex, char* notused);
extern void pthread_mutex_destroy(pthread_mutex_t *mutex);
extern int pthread_mutex_lock(pthread_mutex_t *mutex);
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* WIN32 */

#ifdef ETHER_HEADER_HAS_EA
#  define ESRC(ep) ((ep)->ether_shost.ether_addr_octet)
#  define EDST(ep) ((ep)->ether_dhost.ether_addr_octet)
#else
#  define ESRC(ep) ((ep)->ether_shost)
#  define EDST(ep) ((ep)->ether_dhost)
#endif

/* BSD AF_ values. */
#define BSD_AF_INET             2
#define BSD_AF_INET6_BSD        24      /* OpenBSD (and probably NetBSD), BSD/OS */
#define BSD_AF_INET6_FREEBSD    28
#define BSD_AF_INET6_DARWIN     30

/*
  Courtesy of http://ettercap.sourceforge.net/
*/
#ifndef CFG_LITTLE_ENDIAN
#define ptohs(x) ( (u_int16_t)                       \
                      ((u_int16_t)*((u_int8_t *)x+1)<<8|  \
                      (u_int16_t)*((u_int8_t *)x+0)<<0)   \
                    )

#define ptohl(x) ( (u_int32)*((u_int8_t *)x+3)<<24|  \
                      (u_int32)*((u_int8_t *)x+2)<<16|  \
                      (u_int32)*((u_int8_t *)x+1)<<8|   \
                      (u_int32)*((u_int8_t *)x+0)<<0    \
                    )
#else
#define ptohs(x) *(u_int16_t *)(x)
#define ptohl(x) *(u_int32 *)(x)
#endif

#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define TCPOPT_WSCALE           3
#define TCPOPT_SACKOK           4
#define TCPOPT_TIMESTAMP        8

/* ************************************ */

#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#endif

#ifndef ETHERTYPE_IPV6
#define	ETHERTYPE_IPV6		0x86DD	/* IPv6 protocol */
#endif

#ifndef ETHERTYPE_MPLS
#define	ETHERTYPE_MPLS		0x8847	/* MPLS protocol */
#endif

#ifndef ETHERTYPE_MPLS_MULTI
#define ETHERTYPE_MPLS_MULTI	0x8848	/* MPLS multicast packet */
#endif

struct	ether_mpls_header {
  u_char    label, exp, bos;
  u_char    ttl;
};

#define NULL_HDRLEN             4

/* VLAN support - Courtesy of  Mikael Cam <mca@mgn.net> - 2002/08/28 */
#ifndef ETHER_ADDR_LEN
#define	ETHER_ADDR_LEN	6
#endif

struct	ether_vlan_header {
  u_char    evl_dhost[ETHER_ADDR_LEN];
  u_char    evl_shost[ETHER_ADDR_LEN];
  u_int16_t evl_encap_proto;
  u_int16_t evl_tag;
  u_int16_t evl_proto;
};

#ifndef ETHERTYPE_VLAN
#define	ETHERTYPE_VLAN		0x08100
#endif

typedef struct ipV4Fragment {
  u_int32_t src, dst;
  u_short fragmentId, numPkts, len, sport, dport;
  time_t firstSeen;
  struct ipV4Fragment *next;
} IpV4Fragment;

/* ************************************ */

#define TRANSPORT_UDP          1
#define TRANSPORT_TCP          2
#define TRANSPORT_SCTP         3
#ifdef IP_HDRINCL
#define TRANSPORT_UDP_RAW      4
#endif

typedef struct collectorAddress {
  u_char isIP; /* 0=IPv4, 1=IPv6 or anything else (generic addrinfo) */		  
  u_char transport; /* TRANSPORT_XXXX */
  u_int  flowSequence;

  union {
    struct sockaddr_in v4Address;
#ifndef IPV4_ONLY
    struct {
      struct sockaddr_storage ip;
      u_int len;
    } IPAddress;
#endif
  } u;

  int sockFd; /* Socket file descriptor */
  struct timeval lastExportTime; /* Time when last packet was exported [Set only with -e] */
} CollectorAddress;

/* ************************************ */

#ifndef WIN32
#include <pthread.h>

typedef struct conditionalVariable {
  pthread_mutex_t mutex;
  pthread_cond_t  condvar;
  int predicate;
} ConditionalVariable;

#else

typedef struct conditionalVariable {
  HANDLE condVar;
  CRITICAL_SECTION criticalSection;
} ConditionalVariable;

#endif

extern int createCondvar(ConditionalVariable *condvarId);
extern void deleteCondvar(ConditionalVariable *condvarId);
extern int waitCondvar(ConditionalVariable *condvarId);
extern int signalCondvar(ConditionalVariable *condvarId, int broadcast);

#define TEMP_PREFIX        ".temp"
#define BUF_SIZE           512

#define TRACE_ERROR     0, __FILE__, __LINE__
#define TRACE_WARNING   1, __FILE__, __LINE__
#define TRACE_NORMAL    2, __FILE__, __LINE__
#define TRACE_INFO      3, __FILE__, __LINE__

/* ************************************************ */

extern char *optarg;
extern u_char hasSrcMacExport, srcMacExport[6];

/* ************************************************ */

#ifdef WIN32
typedef float Counter;
#else
typedef unsigned long long Counter;
#endif

/* ********** ICMP ******************** */

#ifdef WIN32

struct icmp_ra_addr
{
  u_int32_t ira_addr;
  u_int32_t ira_preference;
};

struct icmp
{
  u_int8_t  icmp_type;	/* type of message, see below */
  u_int8_t  icmp_code;	/* type sub code */
  u_int16_t icmp_cksum;	/* ones complement checksum of struct */
  union
  {
    u_char ih_pptr;		/* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr;	/* gateway address */
    struct ih_idseq		/* echo datagram */
    {
      u_int16_t icd_id;
      u_int16_t icd_seq;
    } ih_idseq;
    u_int32_t ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu
    {
      u_int16_t ipm_void;
      u_int16_t ipm_nextmtu;
    } ih_pmtu;

    struct ih_rtradv
    {
      u_int8_t irt_num_addrs;
      u_int8_t irt_wpa;
      u_int16_t irt_lifetime;
    } ih_rtradv;
  } icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
  union
  {
    struct
    {
      u_int32_t its_otime;
      u_int32_t its_rtime;
      u_int32_t its_ttime;
    } id_ts;
    struct
    {
      struct ip idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    struct icmp_ra_addr id_radv;
    u_int32_t   id_mask;
    u_int8_t    id_data[1];
  } icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};
#endif /* WIN32 */

/*
 * Definition of ICMP types and code field values.
 */
#define	NPROBE_ICMP_ECHOREPLY		0		/* echo reply */
#define	NPROBE_ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		NPROBE_ICMP_UNREACH_NET	0		/* bad net */
#define		NPROBE_ICMP_UNREACH_HOST	1		/* bad host */
#define		NPROBE_ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		NPROBE_ICMP_UNREACH_PORT	3		/* bad port */
#define		NPROBE_ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		NPROBE_ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define		NPROBE_ICMP_UNREACH_NET_UNKNOWN 6		/* unknown net */
#define		NPROBE_ICMP_UNREACH_HOST_UNKNOWN 7		/* unknown host */
#define		NPROBE_ICMP_UNREACH_ISOLATED	8		/* src host isolated */
#define		NPROBE_ICMP_UNREACH_NET_PROHIB	9		/* prohibited access */
#define		NPROBE_ICMP_UNREACH_HOST_PROHIB 10		/* ditto */
#define		NPROBE_ICMP_UNREACH_TOSNET	11		/* bad tos for net */
#define		NPROBE_ICMP_UNREACH_TOSHOST	12		/* bad tos for host */
#define		NPROBE_ICMP_UNREACH_FILTER_PROHIB 13		/* admin prohib */
#define		NPROBE_ICMP_UNREACH_HOST_PRECEDENCE 14		/* host prec vio. */
#define		NPROBE_ICMP_UNREACH_PRECEDENCE_CUTOFF 15	/* prec cutoff */
#define	NPROBE_ICMP_SOURCEQUENCH	4		/* packet lost, slow down */
#define	NPROBE_ICMP_REDIRECT		5		/* shorter route, codes: */
#define		NPROBE_ICMP_REDIRECT_NET	0		/* for network */
#define		NPROBE_ICMP_REDIRECT_HOST	1		/* for host */
#define		NPROBE_ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define		NPROBE_ICMP_REDIRECT_TOSHOST	3		/* for tos and host */
#define	NPROBE_ICMP_ECHO		8		/* echo service */
#define	NPROBE_ICMP_ROUTERADVERT	9		/* router advertisement */
#define	NPROBE_ICMP_ROUTERSOLICIT	10		/* router solicitation */
#define	NPROBE_ICMP_TIMXCEED		11		/* time exceeded, code: */
#define		NPROBE_ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		NPROBE_ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */
#define	NPROBE_ICMP_PARAMPROB		12		/* ip header bad */
#define		NPROBE_ICMP_PARAMPROB_ERRATPTR 0		/* error at param ptr */
#define		NPROBE_ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */
#define		NPROBE_ICMP_PARAMPROB_LENGTH 2			/* bad length */
#define	NPROBE_ICMP_TSTAMP		13		/* timestamp request */
#define	NPROBE_ICMP_TSTAMPREPLY	14		/* timestamp reply */
#define	NPROBE_ICMP_IREQ		15		/* information request */
#define	NPROBE_ICMP_IREQREPLY		16		/* information reply */
#define	NPROBE_ICMP_MASKREQ		17		/* address mask request */
#define	NPROBE_ICMP_MASKREPLY		18		/* address mask reply */

#define	NPROBE_ICMP_MAXTYPE		18

/* ********* NETFLOW ****************** */

#ifdef WIN32
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef _int64 u_int64_t;
#endif

/*
  For more info see:

  http://www.cisco.com/warp/public/cc/pd/iosw/ioft/neflct/tech/napps_wp.htm

  ftp://ftp.net.ohio-state.edu/users/maf/cisco/
*/

/* ***************************************** */

#define FLOW_VERSION_5		 5
#define V5FLOWS_PER_PAK		30

struct flow_ver5_hdr {
  u_int16_t version;         /* Current version=5*/
  u_int16_t count;           /* The number of records in PDU. */
  u_int32_t sysUptime;       /* Current time in msecs since router booted */
  u_int32_t unix_secs;       /* Current seconds since 0000 UTC 1970 */
  u_int32_t unix_nsecs;      /* Residual nanoseconds since 0000 UTC 1970 */
  u_int32_t flow_sequence;   /* Sequence number of total flows seen */
  u_int8_t  engine_type;     /* Type of flow switching engine (RP,VIP,etc.)*/
  u_int8_t  engine_id;       /* Slot number of the flow switching engine */
  u_int16_t sampleRate;      /* Packet capture sample rate */
};

struct flow_ver5_rec {
  u_int32_t srcaddr;    /* Source IP Address */
  u_int32_t dstaddr;    /* Destination IP Address */
  u_int32_t nexthop;    /* Next hop router's IP Address */
  u_int16_t input;      /* Input interface index */
  u_int16_t output;     /* Output interface index */
  u_int32_t dPkts;      /* Packets sent in Duration (milliseconds between 1st
			   & last packet in this flow)*/
  u_int32_t dOctets;    /* Octets sent in Duration (milliseconds between 1st
			   & last packet in  this flow)*/
  u_int32_t First;      /* SysUptime at start of flow */
  u_int32_t Last;       /* and of last packet of the flow */
  u_int16_t srcport;    /* TCP/UDP source port number (.e.g, FTP, Telnet, etc.,or equivalent) */
  u_int16_t dstport;    /* TCP/UDP destination port number (.e.g, FTP, Telnet, etc.,or equivalent) */
  u_int8_t pad1;        /* pad to word boundary */
  u_int8_t tcp_flags;   /* Cumulative OR of tcp flags */
  u_int8_t prot;        /* IP protocol, e.g., 6=TCP, 17=UDP, etc... */
  u_int8_t tos;         /* IP Type-of-Service */
  u_int16_t src_as;     /* source peer/origin Autonomous System */
  u_int16_t dst_as;     /* dst peer/origin Autonomous System */
  u_int8_t src_mask;    /* source route's mask bits */
  u_int8_t dst_mask;    /* destination route's mask bits */
  u_int16_t pad2;       /* pad to word boundary */
};

typedef struct single_flow_ver5_rec {
  struct flow_ver5_hdr flowHeader;
  struct flow_ver5_rec flowRecord[V5FLOWS_PER_PAK+1 /* safe against buffer overflows */];
} NetFlow5Record;

/* ************************************ */

#define IN_PAYLOAD_ID         96
#define OUT_PAYLOAD_ID        97

/* NetFlow v9/IPFIX */

typedef struct flow_ver9_hdr {
  u_int16_t version;         /* Current version=9*/
  u_int16_t count;           /* The number of records in PDU. */
  u_int32_t sysUptime;       /* Current time in msecs since router booted */
  u_int32_t unix_secs;       /* Current seconds since 0000 UTC 1970 */
  u_int32_t flow_sequence;   /* Sequence number of total flows seen */
  u_int32_t sourceId;        /* Source id */
} V9FlowHeader; 

typedef struct flow_ver9_template_field {
  u_int16_t fieldType;
  u_int16_t fieldLen;
} V9TemplateField;

typedef struct flow_ver9_template {
  u_int16_t templateFlowset; /* = 0 */
  u_int16_t flowsetLen;
  u_int16_t templateId;
  u_int16_t fieldCount;
} V9Template;

typedef struct flow_ver9_option_template {
  u_int16_t templateFlowset; /* = 0 */
  u_int16_t flowsetLen;
  u_int16_t templateId;
  u_int16_t optionScopeLen;
  u_int16_t optionLen;
} V9OptionTemplate;

typedef struct flow_ver9_flow_set {
  u_int16_t templateId;
  u_int16_t flowsetLen;
} V9FlowSet;

typedef struct flow_ver9_templateids {
  u_int16_t templateId;
  u_int16_t templateLen;
  char      *templateName, *templateDescr;
} V9TemplateId;


#define NETFLOW_MAX_BUFFER_LEN 1440
#define MAX_EXPORT_QUEUE_LEN   65536

#define ACT_NUM_PCAP_THREADS     1
#define MAX_NUM_PCAP_THREADS     8

/* It must stay here as it needs the definition of v9 types */
#include "engine.h"
#include "util.h"
#include "ixp.h"

extern struct timeval initialSniffTime, lastExportTime;

/* ************************************ */

/*

############################################################################
#                                                                          #
# The fingerprint database has the following structure:                    #
#                                                                          #
# WWWW:MSS:TTL:WS:S:N:D:T:F:LEN:OS                                         #
#                                                                          #
# WWWW: 4 digit hex field indicating the TCP Window Size                   #
# MSS : 4 digit hex field indicating the TCP Option Maximum Segment Size   #
#       if omitted in the packet or unknown it is "_MSS"                   #
# TTL : 2 digit hex field indicating the IP Time To Live                   #
# WS  : 2 digit hex field indicating the TCP Option Window Scale           #
#       if omitted in the packet or unknown it is "WS"                     #
# S   : 1 digit field indicating if the TCP Option SACK permitted is true  #
# N   : 1 digit field indicating if the TCP Options contain a NOP          #
# D   : 1 digit field indicating if the IP Don't Fragment flag is set      #
# T   : 1 digit field indicating if the TCP Timestamp is present           #
# F   : 1 digit ascii field indicating the flag of the packet              #
#       S = SYN                                                            #
#       A = SYN + ACK                                                      #
# LEN : 2 digit hex field indicating the length of the packet              #
#       if irrilevant or unknown it is "LT"                                #
# OS  : an ascii string representing the OS                                #
#                                                                          #
# IF YOU FIND A NEW FINGERPRING, PLEASE MAIL IT US WITH THE RESPECTIVE OS  #
# or use the appropriate form at:                                          #
#    http://ettercap.sourceforge.net/index.php?s=stuff&p=fingerprint       #
#                                                                          #
# TO GET THE LATEST DATABASE:                                              #
#                                                                          #
#    http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/~checkout~/ettercap/   #
#           ettercap/etter.passive.os.fp?rev=HEAD&content-type=text/plain  #
#                                                                          #
############################################################################
*/


#define MAX_PAYLOAD_LEN         1400
#define MAX_HASH_MUTEXES          32

/* ************************************ */

struct mypcap {
  int fd, snapshot, linktype, tzoff, offset;
  FILE *rfile;

  /* Other fields have been skipped. Please refer
     to pcap-int.h for the full datatype.
  */
};

/* ******** ANY (Linux) ************ */

#ifndef DLT_ANY
#define DLT_ANY 113
#endif

typedef struct anyHeader {
  u_int16_t  pktType;
  u_int16_t  llcAddressType;
  u_int16_t  llcAddressLen;
  u_char     ethAddress[6];
  u_int16_t  pad;
  u_int16_t  protoType;
} AnyHeader;

/* ************************************ */

#define LONG_SNAPLEN    1600
#define DEFAULT_SNAPLEN  128

struct queued_packet {
  struct pcap_pkthdr h;
  u_char p[DEFAULT_SNAPLEN+1];
};

#define PKT_QUEUE_LEN 32


#define DUMP_TIMEOUT    30 /* seconds */

/* #define DEBUG  */

#define HASH_SIZE       4096 /* buckets */

#ifndef WIN32
#define USE_SYSLOG 1
#else
#undef USE_SYSLOG
#endif

extern int getopt(int num, char *const *argv, const char *opts);
extern char *optarg;

/* *************************** */

extern u_int16_t inputInterfaceIndex, outputInterfaceIndex;
extern u_short idleTimeout, lifetimeTimeout, scanCycle;
extern u_int hashSize;
extern u_int bucketsLeft, globalFlowSequence, hashSize;
extern u_int bucketsAdded, bucketsFreed;
extern HashBucket **theHash, *purgedBuckets;
extern pthread_mutex_t statsMutex;
extern u_short maxPayloadLen;
extern int traceLevel;
#ifndef WIN32
extern int useSyslog;
#endif
extern u_char ignoreAS;
extern char shutdownInProgress;
extern u_int32_t totBytesExp, totExpPktSent, totFlowExp;
extern u_char useIpV6;

/* version.c */
extern char *version, *osName, *buildDate;

/* **************************************************************** */

struct ip_header {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int	ihl:4,		/* header length */
		version:4;			/* version */
#else
	u_int	version:4,			/* version */
		ihl:4;		/* header length */
#endif
	u_char	tos;			/* type of service */
	u_short	tot_len;			/* total length */
	u_short	id;			/* identification */
	u_short	frag_off;			/* fragment offset field */
	u_char	ttl;			/* time to live */
	u_char	protocol;			/* protocol */
	u_short	check;			/* checksum */
        u_int32_t saddr, daddr;	/* source and dest address */
};

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
struct udp_header {
	u_short	source;		/* source port */
	u_short	dest;		/* destination port */
	u_short	len;		/* udp length */
	u_short	check;		/* udp checksum */
};

/* ************************************* */

#define NUM_MAC_INTERFACES   8

struct mac_export_if {
  u_char mac_address[6];
  u_int16_t interface_id;
};

extern u_char num_src_mac_export;
extern struct mac_export_if mac_if_match[NUM_MAC_INTERFACES];
extern void exportBucket(HashBucket *myBucket, u_char free_memory);

/* nprobe.c */
extern u_char netFlowVersion;

/* database.c */
extern u_char db_initialized;
extern int exec_sql_query(char *sql, u_char dump_error_if_any);
extern char* get_last_db_error();
extern int init_database(char *db_host, char* user, char *pw, char *db_name);
extern void dump_flow2db(char *buffer, u_int32_t buffer_len);
