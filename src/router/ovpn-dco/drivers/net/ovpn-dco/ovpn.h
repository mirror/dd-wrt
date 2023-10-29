/* SPDX-License-Identifier: GPL-2.0-only */
/* OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPN_H_
#define _NET_OVPN_DCO_OVPN_H_

#include "main.h"
#include "peer.h"
#include "sock.h"
#include "ovpnstruct.h"

#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/file.h>
#include <net/sock.h>
#include <net/addrconf.h>

struct ovpn_struct;
struct net_device;

int ovpn_struct_init(struct net_device *dev);

u16 ovpn_select_queue(struct net_device *dev, struct sk_buff *skb,
		      struct net_device *sb_dev);

void ovpn_keepalive_xmit(struct ovpn_peer *peer);
void ovpn_explicit_exit_notify_xmit(struct ovpn_peer *peer);

netdev_tx_t ovpn_net_xmit(struct sk_buff *skb, struct net_device *dev);

int ovpn_recv(struct ovpn_struct *ovpn, struct ovpn_peer *peer, struct sk_buff *skb);

void ovpn_encrypt_work(struct work_struct *work);
void ovpn_decrypt_work(struct work_struct *work);
int ovpn_napi_poll(struct napi_struct *napi, int budget);

int ovpn_send_data(struct ovpn_struct *ovpn, u32 peer_id, const u8 *data, size_t len);


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

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
#define netdev_set_priv_destructor(_dev, _destructor) \
	(_dev)->destructor = _destructor
#define netdev_set_def_destructor(_dev) \
	(_dev)->destructor = free_netdev
#else
#define netdev_set_priv_destructor(_dev, _destructor) \
	(_dev)->needs_free_netdev = true; \
	(_dev)->priv_destructor = (_destructor);
#define netdev_set_def_destructor(_dev) \
	(_dev)->needs_free_netdev = true;
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
static inline void skb_mark_not_on_list(struct sk_buff *skb)
{
	skb->next = NULL;
}
#endif

#ifndef NLA_POLICY_MIN_LEN
#define NLA_POLICY_MIN_LEN(_len) {		\
	.type = NLA_BINARY			\
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
#undef NLA_POLICY_NESTED
#undef NLA_POLICY_NESTED_ARRAY
#define _NLA_POLICY_NESTED(maxattr, policy) \
	{ .type = NLA_NESTED, .len = maxattr }
#define _NLA_POLICY_NESTED_ARRAY(maxattr, policy) \
	{ .type = NLA_NESTED_ARRAY, .len = maxattr }
#define NLA_POLICY_NESTED(policy) \
	_NLA_POLICY_NESTED(ARRAY_SIZE(policy) - 1, policy)
#define NLA_POLICY_NESTED_ARRAY(policy) \
	_NLA_POLICY_NESTED_ARRAY(ARRAY_SIZE(policy) - 1, policy)
#endif /* < 5.1 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
enum nla_policy_validation {
	NLA_VALIDATE_NONE,
	NLA_VALIDATE_RANGE,
	NLA_VALIDATE_MIN,
	NLA_VALIDATE_MAX,
	NLA_VALIDATE_FUNCTION,
};

#define NLA_POLICY_EXACT_LEN(_len)	{ .type = NLA_EXACT_LEN, .len = _len }
#define NLA_POLICY_EXACT_LEN_WARN(_len)	{ .type = NLA_EXACT_LEN_WARN, \
					  .len = _len }

#define NLA_POLICY_ETH_ADDR		NLA_POLICY_EXACT_LEN(ETH_ALEN)
#define NLA_POLICY_ETH_ADDR_COMPAT	NLA_POLICY_EXACT_LEN_WARN(ETH_ALEN)

#define __NLA_ENSURE(condition) (sizeof(char[1 - 2*!(condition)]) - 1)
#define NLA_ENSURE_INT_TYPE(tp)				\
	(__NLA_ENSURE(tp == NLA_S8 || tp == NLA_U8 ||	\
		      tp == NLA_S16 || tp == NLA_U16 ||	\
		      tp == NLA_S32 || tp == NLA_U32 ||	\
		      tp == NLA_S64 || tp == NLA_U64) + tp)
#define NLA_ENSURE_NO_VALIDATION_PTR(tp)		\
	(__NLA_ENSURE(tp != NLA_BITFIELD32 &&		\
		      tp != NLA_REJECT &&		\
		      tp != NLA_NESTED &&		\
		      tp != NLA_NESTED_ARRAY) + tp)

#define NLA_POLICY_RANGE(tp, _min, _max) {		\
	.type = NLA_ENSURE_INT_TYPE(tp),		\
}

#define NLA_POLICY_MIN(tp, _min) {			\
	.type = NLA_ENSURE_INT_TYPE(tp),		\
}

#define NLA_POLICY_MAX(tp, _max) {			\
	.type = NLA_ENSURE_INT_TYPE(tp),		\
}

#define NLA_POLICY_VALIDATE_FN(tp, fn, ...) {		\
	.type = NLA_ENSURE_NO_VALIDATION_PTR(tp),	\
	.validate = fn,					\
	.len = __VA_ARGS__ + 0,				\
}
#endif /* < 4.20 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14, 0)


static inline int nlmsg_parse_deprecated(const struct nlmsghdr *nlh, int hdrlen,
					 struct nlattr *tb[], int maxtype,
					 const struct nla_policy *policy,
					 void *extack)
{
	return nlmsg_parse(nlh, hdrlen, tb, maxtype, policy);
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)


static inline int nlmsg_parse_deprecated(const struct nlmsghdr *nlh, int hdrlen,
					 struct nlattr *tb[], int maxtype,
					 const struct nla_policy *policy,
					 void *extack)
{
	return nlmsg_parse(nlh, hdrlen, tb, maxtype, policy,extack);
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
#define sk_wait_event2(__sk, __timeo, __condition, __wait)		\
	({	int __rc;						\
		release_sock(__sk);					\
		__rc = __condition;					\
		if (!__rc) {						\
			*(__timeo) = wait_woken(__wait,			\
						TASK_INTERRUPTIBLE,	\
						*(__timeo));		\
		}							\
		sched_annotate_sleep();					\
		lock_sock(__sk);					\
		__rc = __condition;					\
		__rc;							\
	})


#else
#define sk_wait_event2 sk_wait_event
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
static inline void netif_tx_napi_add(struct net_device *dev,
				     struct napi_struct *napi,
				     int (*poll)(struct napi_struct *, int),
				     int weight)
{
	netif_napi_add(dev, napi, poll, weight);
}
static inline void *skb_put_data(struct sk_buff *skb, const void *data,
				 unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memcpy(tmp, data, len);

	return tmp;
}


#endif /* < 4.5 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
#define __ro_after_init
#endif

#endif /* _NET_OVPN_DCO_OVPN_H_ */
