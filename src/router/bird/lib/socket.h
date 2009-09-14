/*
 *	BIRD Socket Interface
 *
 *	(c) 1998--2004 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SOCKET_H_
#define _BIRD_SOCKET_H_

#include "lib/resource.h"

typedef struct birdsock {
  resource r;
  pool *pool;				/* Pool where incoming connections should be allocated (for SK_xxx_PASSIVE) */
  int type;				/* Socket type */
  void *data;				/* User data */
  ip_addr saddr, daddr;			/* IPA_NONE = unspecified */
  unsigned sport, dport;		/* 0 = unspecified (for IP: protocol type) */
  int tos;				/* TOS and priority, -1 = default */
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

  ip_addr faddr;			/* For packet protocols: source of current packet */
  unsigned fport;

  int fd;				/* System-dependent data */
  node n;
  void *rbuf_alloc, *tbuf_alloc;
  char *password;				/* Password for MD5 authentication */
} sock;

sock *sk_new(pool *);			/* Allocate new socket */
int sk_open(sock *);			/* Open socket */
int sk_send(sock *, unsigned len);	/* Send data, <0=err, >0=ok, 0=sleep */
int sk_send_to(sock *, unsigned len, ip_addr to, unsigned port); /* sk_send to given destination */
void sk_reallocate(sock *);		/* Free and allocate tbuf & rbuf */
void sk_dump_all(void);
int sk_set_ttl(sock *s, int ttl);	/* Set TTL for given socket */
int sk_set_md5_auth(sock *s, ip_addr a, char *passwd);	/* Add or remove security associations for given passive socket */

static inline int
sk_send_buffer_empty(sock *sk)
{
	return sk->tbuf == sk->tpos;
}


/* Socket flags */

#define SKF_V6ONLY	1	/* Use  IPV6_V6ONLY socket option */


/*
 *	Socket types		     SA SP DA DP IF  TTL SendTo	(?=may, -=must not, *=must)
 */

#define SK_TCP_PASSIVE	0	   /* ?  *  -  -  -  ?   -	*/
#define SK_TCP_ACTIVE	1          /* ?  ?  *  *  -  ?   -	*/
#define SK_TCP		2
#define SK_UDP		3          /* ?  ?  -  -  -  ?   ?	*/
#define SK_UDP_MC       4          /* ?  ?  *  *  *  *   -	*/
#define SK_IP		5          /* ?  -  -  *  -  ?   ?	*/
#define SK_IP_MC	6          /* ?  -  *  *  *  *   -	*/
#define SK_MAGIC	7	   /* Internal use by sysdep code */
#define SK_UNIX_PASSIVE	8
#define SK_UNIX		9

/*
 *  Multicast sockets are slightly different from the other ones:
 *  If you want to send packets only, just set the destination
 *  address to the corresponding multicast group and iface to
 *  the interface to be used. If you also want receiving, set
 *  source address to the same multicast group as well.
 */

#endif
