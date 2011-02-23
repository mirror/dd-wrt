/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/common.h - Common Definitions
***********************************************************
 $Author: jeroen $
 $Id: common.h,v 1.23 2007-01-11 14:50:51 jeroen Exp $
 $Date: 2007-01-11 14:50:51 $
**********************************************************/

#ifndef AICCU_COMMON_H
#define AICCU_COMMON_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include <sys/types.h>

#ifdef _DEBUG
#define D(x) x
#else
#define D(x) {}
#endif

#ifndef _OPENBSD
#ifndef _SUNOS
#ifndef _AIX
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#endif
#endif
#endif
#define __STRICT_ANSI__

/* Don't deprecate strncat etc. */
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef _AIX
#define _H_ARPA_ONAMESER_COMPAT "AICCU workaround"
#include <net/net_globals.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(_SUNOS) || defined(_AIX) || defined(_DARWIN)
/* Include this as it knows quite a bit about endianess */
#include <arpa/nameser_compat.h>
#else
#ifndef _WIN32
#if defined(_OPENBSD) || defined(_DFBSD) || defined(_FREEBSD) || defined(_NETBSD)
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#endif
#endif

/* For MD5 routines */
#define __USE_BSD 1
#include <sys/types.h>

#ifndef _WIN32
	/* Unix Specifics */

#ifndef linux
	#include <netinet/in_systm.h>
#endif

#ifdef _DARWIN
#define _BSD_SOCKLEN_T_
#endif

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <strings.h>
	#include <syslog.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <sys/select.h>

	#include <net/if.h>
	#include <netinet/if_ether.h>
#ifdef linux
	#include <netpacket/packet.h>
	#include <linux/if_tun.h>
#else
#ifdef _DFBSD
	#include <net/tun/if_tun.h>
#else
#ifdef _DARWIN
	/*
	 * Darwin doesn't have TUN/TAP support per default
	 * It is available from http://www-user.rhrk.uni-kl.de/~nissler/tuntap/
	 * which is a port made by Mattias Nissler
	 * for compiling convienience we have included the ioctl's here
	 */
	#define TUNSIFHEAD  _IOW('t', 96, int)
	#define TUNGIFHEAD  _IOR('t', 97, int)
#else
#ifndef _AIX
	#include <net/if_tun.h>
/* endif for !_AIX */
#endif
/* endif for _DARWIN else */
#endif
/* endif for _DFBSD else */
#endif
/* endif for linux else */
#endif
	#include <netinet/ip.h>
	#include <netinet/ip6.h>
	#include <netinet/icmp6.h>
	#include <netinet/tcp.h>
	#include <netinet/udp.h>
	#include <netinet/ip_icmp.h>
	#include <sys/ioctl.h>

#if defined(_OPENBSD) || defined(_DARWIN) || defined(_FREEBSD) || defined(_DFBSD)
	#include <sys/uio.h>
#endif

	#include <pthread.h>

	/*
	 * Windows abstracts sockets to a different
	 * type, as this is actually pretty nice
	 * we'll do it too
	 */
	#ifndef SOCKET
		typedef int SOCKET;
	#endif

	/* closesocket() -> close() on unices */
	#define closesocket close

	/*
	 * Expect a BSD style in6_addr who puts
	 * this between a #ifdef _KERNEL...
	 * Probably against people doing stuff in userspace?
	 */
	#ifndef s6_addr
	#ifndef _SUNOS
	#define s6_addr __u6_addr.__u6_addr8
	#else
	#define s6_addr _S6_un._S6_u8
	#endif
	#endif
#else

	/* Winsock */
	#include <winsock2.h>
	#include <ws2tcpip.h>

	/* Windows Specifics */
	#include <io.h>

/*
	 * Some weird M$ person thought it was
	 * funny to underscore common functions !?
	 */
	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strcasecmp	_stricmp
	#define strncasecmp	_strnicmp
	#define strdup		_strdup

	/* 
	 * Capitalize this one
	 * Sleep() is in milliseconds
	 */
	#define sleep(x) Sleep(x*1000)

	/* No syslog on Windows */
	#define LOG_DEBUG 1
	#define LOG_ERR 2
	#define LOG_WARNING 3
	#define LOG_INFO 4

	typedef unsigned long u_int32_t;
	typedef unsigned long long u_int64_t;

	typedef unsigned char u_int8_t;
	typedef unsigned __int16 u_int16_t;
	typedef unsigned __int64 u_int64_t;

	/* Not available in the Winsock2 includes */
	#define IPPROTO_NONE 59 /* IPv6 no next header */

	#define BIG_ENDIAN 4321
	#define LITTLE_ENDIAN 1234

	#define __BIG_ENDIAN BIG_ENDIAN
	#define __LITTLE_ENDIAN LITTLE_ENDIAN

	/* Fix byte order */
	#define __BYTE_ORDER __LITTLE_ENDIAN
	#define BYTE_ORDER LITTLE_ENDIAN

	#define s6_addr16	_S6_un.Word
	#define SHUT_RDWR	SD_BOTH
	#define uint8_t		u_int8_t
	#define uint16_t	u_int16_t
	#define uint32_t	u_int32_t
	#define uint64_t	u_int64_t

	struct ether
	{
		uint16_t	ether_dhost[3];
		uint16_t	ether_shost[3];
		uint16_t	ether_type;
	};

	/* The IPv6 Header */	
	struct ip6_hdr
	{
		union
		{
			struct ip6_hdrctl
			{
				uint32_t	ip6_un1_flow;	/* 4 bits version, 8 bits TC, 20 bits flow-ID */
				uint16_t	ip6_un1_plen;	/* payload length */
				uint8_t		ip6_un1_nxt;	/* next header */
				uint8_t		ip6_un1_hlim;	/* hop limit */
			}			ip6_un1;
			uint8_t			ip6_un2_vfc;	/* 4 bits version, top 4 bits tclass */
		}				ip6_ctlun;
		struct in6_addr ip6_src;			/* source address */
		struct in6_addr ip6_dst;			/* destination address */
	};

	/* ICMPv6 */
	struct icmp6_hdr
	{
		uint8_t			icmp6_type;		/* type field */
		uint8_t			icmp6_code;		/* code field */
		uint16_t		icmp6_cksum;		/* checksum field */
		union
		{
			uint32_t	icmp6_un_data32[1];	/* type-specific field */
			uint16_t	icmp6_un_data16[2];	/* type-specific field */
			uint8_t		icmp6_un_data8[4];	/* type-specific field */
		} icmp6_dataun;
	};

	#define ND_NEIGHBOR_SOLICIT         135
	#define ND_NEIGHBOR_ADVERT          136

	struct nd_neighbor_solicit
	{
		struct in6_addr		nd_ns_target;		/* target address */
								/* could be followed by options */
	};

	struct nd_neighbor_advert
	{
		struct in6_addr		nd_na_target;		/* target address */
								/* could be followed by options */
		uint8_t			nd_no_type;		/* Option providing the target MAC address */
		uint8_t			nd_no_len;		/* Length (1) */
		uint8_t			nd_no_mac[6];		/* MAC address */

	};

	const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
	int inet_pton(int af, const char *src, void *dst);
#endif /* WIN32 */


#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL	0
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN  __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN     __BIG_ENDIAN
#endif
#ifndef PDP_ENDIAN
#define PDP_ENDIAN     __PDP_ENDIAN
#endif
#ifndef BYTE_ORDER
#define BYTE_ORDER     __BYTE_ORDER
#endif

/* Boolean support */
#ifndef bool
#define bool uint32_t
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/* Include MD5 and SHA1 support */
#include "hash_md5.h"
#include "hash_sha1.h"

/* Resolver includes */
#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#endif

#ifdef AICCU_GNUTLS
#include <gnutls/gnutls.h>
#endif

#ifndef NS_GET16SZ
#define NS_INT32SZ      4       /* #/bytes of data in a u_int32_t */
#endif

#ifndef NS_GET32SZ
#define NS_INT16SZ      2       /* #/bytes of data in a u_int16_t */
#endif


#ifndef NS_GET16
#define NS_GET16(s, cp) do { \
        register u_char *t_cp = (u_char *)(cp); \
        (s) = ((u_int16_t)t_cp[0] << 8) \
            | ((u_int16_t)t_cp[1]) \
            ; \
        (cp) += NS_INT16SZ; \
} while (0)
#endif

#ifndef NS_GET32
#define NS_GET32(l, cp) do { \
        register u_char *t_cp = (u_char *)(cp); \
        (l) = ((u_int32_t)t_cp[0] << 24) \
            | ((u_int32_t)t_cp[1] << 16) \
            | ((u_int32_t)t_cp[2] << 8) \
            | ((u_int32_t)t_cp[3]) \
            ; \
        (cp) += NS_INT32SZ; \
} while (0)
#endif

/* parseline() rules */
enum pl_ruletype
{
	PLRT_STRING,		/* Offset points to a String (strdup()) */
	PLRT_INTEGER,		/* Offset points to a Integer (unsigned int) */
	PLRT_BOOL,		/* Offset points to a Boolean. */
	PLRT_IPV4,		/* Offset points to a IPv4 address (inet_pton(..., AF_INET)) */
	PLRT_IPV6,		/* Offset points to a IPv6 address (inet_pton(..., AF_INET6)) */
	PLRT_END		/* End of rules */
};

struct pl_rule
{
	const char		*title;
	unsigned int		type;
	unsigned int		offset;
};


struct tlssocket
{
	SOCKET			socket;
#ifdef AICCU_GNUTLS
	bool			tls_active;	/* TLS active? */
	gnutls_session		session;	/* The GnuTLS sesision */
#endif
};

typedef struct tlssocket * TLSSOCKET;

/* Common Functions */
void dologA(int level, const char *fmt, va_list ap);
void dolog(int level, const char *fmt, ...);

#ifdef _AIX
void vsyslog(int priority, const char *format, va_list ap);
#endif

/* Networking functions */
void sock_printf(TLSSOCKET sock, const char *fmt, ...);
int sock_getline(TLSSOCKET sock, char *rbuf, unsigned int rbuflen, unsigned int *filled, char *ubuf, unsigned int ubuflen);
TLSSOCKET connect_client(const char *hostname, const char *service, int family, int socktype);
TLSSOCKET listen_server(const char *description, const char *hostname, const char *service, int family, int socktype);
void sock_free(TLSSOCKET sock);
#ifdef AICCU_GNUTLS
bool sock_gotls(TLSSOCKET sock);
#endif

/* Parsing functions */
unsigned int countfields(char *s);
bool copyfield(char *s, unsigned int n, char *buf, unsigned int buflen);
bool parseline(char *line, const char *split, struct pl_rule *rules, void *data);

/* Convienience */
void MD5String(const char *sString, char *sSignature, unsigned int siglen);
bool is_rfc1918(char *ipv4);

#endif /* AICCU_COMMON_H */
