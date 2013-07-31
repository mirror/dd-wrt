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

#include "lib/resource.h"

typedef struct birdsock {
  resource r;
  pool *pool;				/* Pool where incoming connections should be allocated (for SK_xxx_PASSIVE) */
  int type;				/* Socket type */
  void *data;				/* User data */
  ip_addr saddr, daddr;			/* IPA_NONE = unspecified */
  unsigned sport, dport;		/* 0 = unspecified (for IP: protocol type) */
  int tos;				/* TOS / traffic class, -1 = default */
  int priority;				/* Local socket priority, -1 = default */
  int ttl;				/* Time To Live, -1 = default */
  u32 flags;
  struct iface *iface;			/* Interface; specify this for broad/multicast sockets */

  byte *rbuf, *rpos;			/* NULL=allocate automatically */
  unsigned rbsize;
  int (*rx_hook)(struct birdsock *, int size); /* NULL=receiving turned off, returns 1 to clear rx buffer */

  byte *tbuf, *tpos;			/* NULL=allocate automatically */
  byte *ttx;				/* Internal */
  unsigned tbsize;
  void (*tx_hook)(struct birdsock *);

  void (*err_hook)(struct birdsock *, int); /* errno or zero if EOF */

  /* Information about received datagrams (UDP, RAW), valid in rx_hook */
  ip_addr faddr, laddr;			/* src (From) and dst (Local) address of the datagram */
  unsigned fport;			/* src port of the datagram */
  unsigned lifindex;			/* local interface that received the datagram */
  /* laddr and lifindex are valid only if SKF_LADDR_RX flag is set to request it */

  int fd;				/* System-dependent data */
  node n;
  void *rbuf_alloc, *tbuf_alloc;
  char *password;				/* Password for MD5 authentication */
} sock;

sock *sock_new(pool *);			/* Allocate new socket */
#define sk_new(X) sock_new(X)		/* Wrapper to avoid name collision with OpenSSL */

int sk_open(sock *);			/* Open socket */
int sk_send(sock *, unsigned len);	/* Send data, <0=err, >0=ok, 0=sleep */
int sk_send_to(sock *, unsigned len, ip_addr to, unsigned port); /* sk_send to given destination */
void sk_reallocate(sock *);		/* Free and allocate tbuf & rbuf */
void sk_dump_all(void);
int sk_set_ttl(sock *s, int ttl);	/* Set transmit TTL for given socket */
int sk_set_min_ttl(sock *s, int ttl);	/* Set minimal accepted TTL for given socket */

/* Add or remove security associations for given passive socket */
int sk_set_md5_auth(sock *s, ip_addr a, struct iface *ifa, char *passwd);
int sk_rx_ready(sock *s);

/* Prepare UDP or IP socket to multicasting. s->iface and s->ttl must be set */
int sk_setup_multicast(sock *s);	
int sk_join_group(sock *s, ip_addr maddr);
int sk_leave_group(sock *s, ip_addr maddr);

#ifdef IPV6
int sk_set_ipv6_checksum(sock *s, int offset);
int sk_set_icmp_filter(sock *s, int p1, int p2);
#endif

int sk_set_broadcast(sock *s, int enable);

static inline int
sk_send_buffer_empty(sock *sk)
{
	return sk->tbuf == sk->tpos;
}

extern int sk_priority_control;	/* Suggested priority for control traffic, should be sysdep define */

/* Socket flags */

#define SKF_V6ONLY	1	/* Use IPV6_V6ONLY socket option */
#define SKF_LADDR_RX	2	/* Report local address for RX packets */
#define SKF_LADDR_TX	4	/* Allow to specify local address for TX packets */
#define SKF_TTL_RX	8	/* Report TTL / Hop Limit for RX packets */


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
 */

#endif
