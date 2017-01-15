/*
 *	BIRD Socket Interface
 *
 *	(c) 1998--2004 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SOCKET_H_
#define _BIRD_SOCKET_H_

#include <errno.h>
// #include <sys/socket.h>

#include "lib/resource.h"

typedef struct birdsock {
  resource r;
  pool *pool;				/* Pool where incoming connections should be allocated (for SK_xxx_PASSIVE) */
  int type;				/* Socket type */
  void *data;				/* User data */
  ip_addr saddr, daddr;			/* IPA_NONE = unspecified */
  uint sport, dport;			/* 0 = unspecified (for IP: protocol type) */
  int tos;				/* TOS / traffic class, -1 = default */
  int priority;				/* Local socket priority, -1 = default */
  int ttl;				/* Time To Live, -1 = default */
  u32 flags;
  struct iface *iface;			/* Interface; specify this for broad/multicast sockets */

  byte *rbuf, *rpos;			/* NULL=allocate automatically */
  uint fast_rx;				/* RX has higher priority in event loop */
  uint rbsize;
  int (*rx_hook)(struct birdsock *, uint size); /* NULL=receiving turned off, returns 1 to clear rx buffer */

  byte *tbuf, *tpos;			/* NULL=allocate automatically */
  byte *ttx;				/* Internal */
  uint tbsize;
  void (*tx_hook)(struct birdsock *);

  void (*err_hook)(struct birdsock *, int); /* errno or zero if EOF */

  /* Information about received datagrams (UDP, RAW), valid in rx_hook */
  ip_addr faddr, laddr;			/* src (From) and dst (Local) address of the datagram */
  uint fport;				/* src port of the datagram */
  uint lifindex;			/* local interface that received the datagram */
  /* laddr and lifindex are valid only if SKF_LADDR_RX flag is set to request it */

  int af;				/* Address family (AF_INET, AF_INET6 or 0 for non-IP) of fd */
  int fd;				/* System-dependent data */
  int index;				/* Index in poll buffer */
  int rcv_ttl;				/* TTL of last received datagram */
  node n;
  void *rbuf_alloc, *tbuf_alloc;
  char *password;			/* Password for MD5 authentication */
  char *err;				/* Error message */
} sock;

sock *sock_new(pool *);			/* Allocate new socket */
#define sk_new(X) sock_new(X)		/* Wrapper to avoid name collision with OpenSSL */

int sk_open(sock *);			/* Open socket */
int sk_rx_ready(sock *s);
int sk_send(sock *, uint len);		/* Send data, <0=err, >0=ok, 0=sleep */
int sk_send_to(sock *, uint len, ip_addr to, uint port); /* sk_send to given destination */
void sk_reallocate(sock *);		/* Free and allocate tbuf & rbuf */
void sk_set_rbsize(sock *s, uint val);	/* Resize RX buffer */
void sk_set_tbsize(sock *s, uint val);	/* Resize TX buffer, keeping content */
void sk_set_tbuf(sock *s, void *tbuf);	/* Switch TX buffer, NULL-> return to internal */
void sk_dump_all(void);

static inline int sk_send_buffer_empty(sock *sk)
{ return sk->tbuf == sk->tpos; }


#ifdef IPV6
#define sk_is_ipv4(X) 0
#define sk_is_ipv6(X) 1
#else
#define sk_is_ipv4(X) 1
#define sk_is_ipv6(X) 0
#endif


int sk_setup_multicast(sock *s);	/* Prepare UDP or IP socket for multicasting */
int sk_join_group(sock *s, ip_addr maddr);	/* Join multicast group on sk iface */
int sk_leave_group(sock *s, ip_addr maddr);	/* Leave multicast group on sk iface */
int sk_setup_broadcast(sock *s);
int sk_set_ttl(sock *s, int ttl);	/* Set transmit TTL for given socket */
int sk_set_min_ttl(sock *s, int ttl);	/* Set minimal accepted TTL for given socket */
int sk_set_md5_auth(sock *s, ip_addr local, ip_addr remote, struct iface *ifa, char *passwd, int setkey);
int sk_set_ipv6_checksum(sock *s, int offset);
int sk_set_icmp6_filter(sock *s, int p1, int p2);
void sk_log_error(sock *s, const char *p);

byte * sk_rx_buffer(sock *s, int *len);	/* Temporary */

extern int sk_priority_control;		/* Suggested priority for control traffic, should be sysdep define */


/* Socket flags */

#define SKF_V4ONLY	0x01	/* Use IPv4 for IP sockets */
#define SKF_V6ONLY	0x02	/* Use IPV6_V6ONLY socket option */
#define SKF_LADDR_RX	0x04	/* Report local address for RX packets */
#define SKF_TTL_RX	0x08	/* Report TTL / Hop Limit for RX packets */
#define SKF_BIND	0x10	/* Bind datagram socket to given source address */
#define SKF_HIGH_PORT	0x20	/* Choose port from high range if possible */

#define SKF_THREAD	0x100	/* Socked used in thread, Do not add to main loop */
#define SKF_TRUNCATED	0x200	/* Received packet was truncated, set by IO layer */
#define SKF_HDRINCL	0x400	/* Used internally */
#define SKF_PKTINFO	0x800	/* Used internally */

/*
 *	Socket types		     SA SP DA DP IF  TTL SendTo	(?=may, -=must not, *=must)
 */

#define SK_TCP_PASSIVE	0	   /* ?  *  -  -  -  ?   -	*/
#define SK_TCP_ACTIVE	1          /* ?  ?  *  *  -  ?   -	*/
#define SK_TCP		2
#define SK_UDP		3          /* ?  ?  ?  ?  ?  ?   ?	*/
#define SK_IP		5          /* ?  -  ?  *  ?  ?   ?	*/
#define SK_MAGIC	7	   /* Internal use by sysdep code */
#define SK_UNIX_PASSIVE	8
#define SK_UNIX		9

/*
 *  For SK_UDP or SK_IP sockets setting DA/DP allows to use sk_send(),
 *  otherwise sk_send_to() must be used.
 *
 *  For SK_IP sockets setting DP specifies protocol number, which is used
 *  for both receiving and sending.
 *
 *  For multicast on SK_UDP or SK_IP sockets set IF and TTL,
 *  call sk_setup_multicast() to enable multicast on that socket,
 *  and then use sk_join_group() and sk_leave_group() to manage
 *  a set of received multicast groups.
 *
 *  For datagram (SK_UDP, SK_IP) sockets, there are two ways to handle
 *  source address. The socket could be bound to it using bind()
 *  syscall, but that also forbids the reception of multicast packets,
 *  or the address could be set on per-packet basis using platform
 *  dependent options (but these are not available in some corner
 *  cases). The first way is used when SKF_BIND is specified, the
 *  second way is used otherwise.
 */

#endif
