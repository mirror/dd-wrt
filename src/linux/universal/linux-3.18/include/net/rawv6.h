#ifndef _NET_RAWV6_H
#define _NET_RAWV6_H

#include <net/protocol.h>


#ifdef CONFIG_INET_RAW
void raw6_icmp_error(struct sk_buff *, int nexthdr,
		u8 type, u8 code, int inner_offset, __be32);
bool raw6_local_deliver(struct sk_buff *, int);
#else
static inline void raw6_icmp_error(struct sk_buff *skb, int nexthdr,
		u8 type, u8 code, int inner_offset, __be32 x) {}
static inline bool raw6_local_deliver(struct sk_buff *skb, int x)
{ return false; }
#endif

int rawv6_rcv(struct sock *sk, struct sk_buff *skb);

#if defined(CONFIG_IPV6_MIP6) || defined(CONFIG_IPV6_MIP6_MODULE)
int rawv6_mh_filter_register(int (*filter)(struct sock *sock,
					   struct sk_buff *skb));
int rawv6_mh_filter_unregister(int (*filter)(struct sock *sock,
					     struct sk_buff *skb));
#endif

#endif
