/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 */

#ifndef _NET_BATMAN_ADV_COMPAT_H_
#define _NET_BATMAN_ADV_COMPAT_H_

#ifdef __KERNEL__

#include <linux/version.h>	/* LINUX_VERSION_CODE */
#include <linux/kconfig.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <net/netlink.h>
#include <generated/autoconf.h>

#include "compat-autoconf.h"

#if LINUX_VERSION_IS_LESS(4,1,0)
static inline int nla_put_in_addr(struct sk_buff *skb, int attrtype,
				  __be32 addr)
{
	return nla_put_be32(skb, attrtype, addr);
}

static inline int nla_put_in6_addr(struct sk_buff *skb, int attrtype,
				   const struct in6_addr *addr)
{
	return nla_put(skb, attrtype, sizeof(*addr), addr);
}

static inline __be32 nla_get_in_addr(const struct nlattr *nla)
{
	return *(__be32 *) nla_data(nla);
}

static inline struct in6_addr nla_get_in6_addr(const struct nlattr *nla)
{
	struct in6_addr tmp;

	nla_memcpy(&tmp, nla, sizeof(tmp));
	return tmp;
}
#endif /* < 4.1 */

#if LINUX_VERSION_IS_LESS(4,9,0)
static inline void *__skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	void *tmp = __skb_put(skb, len);

	memset(tmp, 0, len);
	return tmp;
}

static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memset(tmp, 0, len);

	return tmp;
}

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
				 unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memcpy(tmp, data, len);

	return tmp;
}
static inline void skb_put_u8(struct sk_buff *skb, u8 val)
{
	*(u8 *)skb_put(skb, 1) = val;
}
#endif


#if LINUX_VERSION_IS_LESS(4, 15, 0)

#define batadv_softif_slave_add(__dev, __slave_dev, __extack) \
	batadv_softif_slave_add(__dev, __slave_dev)

#endif /* LINUX_VERSION_IS_LESS(4, 15, 0) */

#ifndef from_timer
#define TIMER_DATA_TYPE          unsigned long
#define TIMER_FUNC_TYPE          void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
			       void (*callback) (struct timer_list *),
			       unsigned int flags)
{
#ifdef __setup_timer
	__setup_timer(timer, (TIMER_FUNC_TYPE) callback,
		      (TIMER_DATA_TYPE) timer, flags);
#else
	if (flags & TIMER_DEFERRABLE)
		setup_deferrable_timer(timer, (TIMER_FUNC_TYPE) callback,
				       (TIMER_DATA_TYPE) timer);
	else
		setup_timer(timer, (TIMER_FUNC_TYPE) callback,
			    (TIMER_DATA_TYPE) timer);
#endif
}

#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)
#endif
#ifndef ETH_MIN_MTU
#define ETH_MIN_MTU	68		/* Min IPv4 MTU per RFC791	*/
#endif

#endif /* __KERNEL__ */

#endif /* _NET_BATMAN_ADV_COMPAT_H_ */
