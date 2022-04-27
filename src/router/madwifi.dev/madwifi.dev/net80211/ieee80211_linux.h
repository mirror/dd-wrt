/*-
 * Copyright (c) 2003-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_linux.h 3269 2008-01-26 20:56:20Z mtaylor $
 */
#ifndef _NET80211_IEEE80211_LINUX_H_
#define _NET80211_IEEE80211_LINUX_H_

						/* #define ATH_DEBUG_SPINLOCKS *//* announce before spinlocking */

#include <linux/wireless.h>
#include <linux/fs.h>

/*
 * Compatibility definition of statistics flags
 * (bitmask in (struct iw_quality *)->updated)
 */
#ifndef IW_QUAL_QUAL_UPDATED
#define IW_QUAL_QUAL_UPDATED	0x01	/* Value was updated since last read */
#define IW_QUAL_LEVEL_UPDATED	0x02
#define IW_QUAL_NOISE_UPDATED	0x04
#define IW_QUAL_QUAL_INVALID	0x10	/* Driver doesn't provide value */
#define IW_QUAL_LEVEL_INVALID	0x20
#define IW_QUAL_NOISE_INVALID	0x40
#endif				/* IW_QUAL_QUAL_UPDATED */

#ifndef IW_QUAL_ALL_UPDATED
#define IW_QUAL_ALL_UPDATED \
	(IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED | IW_QUAL_NOISE_UPDATED)
#endif
#ifndef IW_QUAL_ALL_INVALID
#define IW_QUAL_ALL_INVALID \
	(IW_QUAL_QUAL_INVALID | IW_QUAL_LEVEL_INVALID | IW_QUAL_NOISE_INVALID)
#endif

/*
 * The RSSI values reported in the TX/RX descriptors in the driver are the SNR 
 * expressed in dBm. Thus 'rssi' is signal level above the noise floor in dBm.
 *
 * Noise is measured in dBm and is negative unless there is an unimaginable 
 * level of RF noise. 
 *
 * The signal level is noise + rssi.
 *
 * Note that the iw_quality values are 1 byte, and can be signed, unsigned or 
 * negative depending on context.
 *
 */
static __inline void set_quality(struct iw_quality *iq, u_int rssi, int noise)
{
	iq->qual = rssi;
	iq->noise = noise;
	iq->level = ((((int)rssi + noise) <= 0) ? ((int)rssi + noise) : 0);
	iq->updated = IW_QUAL_ALL_UPDATED;
#if WIRELESS_EXT >= 19
	iq->updated |= IW_QUAL_DBM;
#endif
}

#ifndef container_of
#define container_of(ptr, type, member) ({          \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	    (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

/*
 * Task deferral
 *
 * Deduce if tasklets are available.  If not then
 * fall back to using the immediate work queue.
 */
#include <linux/interrupt.h>
#ifdef DECLARE_TASKLET		/* native tasklets */
#define IEEE80211_TQ_STRUCT tasklet_struct
#define IEEE80211_INIT_TQUEUE(a,b,c)	tasklet_init((a), (b), (unsigned long)(c))
#define IEEE80211_SCHEDULE_TQUEUE(a)	tasklet_schedule((a))
#define IEEE80211_CANCEL_TQUEUE(a)	if (!in_interrupt()) tasklet_kill((a))
typedef unsigned long IEEE80211_TQUEUE_ARG;
#define mark_bh(a) do {} while (0)
#else				/* immediate work queue */
#define IEEE80211_TQ_STRUCT tq_struct
#define IEEE80211_INIT_TQUEUE(a,b,c)		INIT_TQUEUE(a,b,c)
#define IEEE80211_SCHEDULE_TQUEUE(a) do { \
	int __macro_needmark;		\
	__macro_needmark |= queue_task((a), &tq_immediate);	\
	if (__macro_needmark)		\
		mark_bh(IMMEDIATE_BH);	\
} while (0)
typedef void *IEEE80211_TQUEUE_ARG;
#define	tasklet_disable(t)	do { (void) t; local_bh_disable(); } while (0)
#define	tasklet_enable(t)	do { (void) t; local_bh_enable(); } while (0)
/* XXX: not supporting cancel in old kernels! */
#define IEEE80211_CANCEL_TQUEUE(a)	((a), 0)
#endif				/* !DECLARE_TASKLET */

#define	IEEE80211_RESCHEDULE	schedule

#include <linux/sched.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,41)
#include <linux/tqueue.h>
#define work_struct			tq_struct
#define schedule_work(t)		schedule_task((t))
#define flush_scheduled_work()		flush_scheduled_tasks()
#define IEEE80211_INIT_WORK(t, f) do { 			\
	memset((t), 0, sizeof(struct tq_struct)); \
	(t)->routine = (void (*)(void*)) (f); 	\
	(t)->data=(void *) (t);			\
} while (0)
#else
#include <linux/workqueue.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
#define IEEE80211_INIT_WORK(_t, _f)	INIT_WORK((_t), (void (*)(void *))(_f), (_t));
#else
#define IEEE80211_INIT_WORK(_t, _f)	INIT_WORK((_t), (_f));
#endif

#endif				/* KERNEL_VERSION < 2.5.41 */

/* Locking */
/* NB: beware, spin_is_locked() is not usefully defined for !(DEBUG || SMP)
 * because spinlocks do not exist in this configuration. Instead IRQs 
 * or pre-emption are simply disabled, as this is all that is needed.
 */

/*
 * Beacon handler locking definitions.
 * Beacon locking 
 * UAPSD locking 
 */
typedef spinlock_t ieee80211com_lock_t;
#define	IEEE80211_LOCK_INIT(_ic, _name)					\
	spin_lock_init(&(_ic)->ic_comlock)
#define	IEEE80211_LOCK_DESTROY(_ic)
#define	IEEE80211_LOCK_IRQ(_ic) do {					\
	unsigned long __ilockflags;					\
	IEEE80211_LOCK_CHECK(_ic);					\
	spin_lock_irqsave(&(_ic)->ic_comlock, __ilockflags);
#define	IEEE80211_UNLOCK_IRQ(_ic)					\
	IEEE80211_LOCK_ASSERT(_ic);					\
	spin_unlock_irqrestore(&(_ic)->ic_comlock, __ilockflags);	\
} while (0)
#define	IEEE80211_UNLOCK_IRQ_EARLY(_ic)					\
	IEEE80211_LOCK_ASSERT(_ic);					\
	spin_unlock_irqrestore(&(_ic)->ic_comlock, __ilockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	IEEE80211_LOCK_ASSERT(_ic) \
	KASSERT(spin_is_locked(&(_ic)->ic_comlock), ("ieee80211com not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	IEEE80211_LOCK_CHECK(_ic) do { \
	if (spin_is_locked(&(_ic)->ic_comlock)) \
		printk("%s:%d - about to block on ieee80211com lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	IEEE80211_LOCK_CHECK(_ic)
#endif
#else
#define	IEEE80211_LOCK_ASSERT(_ic)
#define	IEEE80211_LOCK_CHECK(_ic)
#endif

#define IEEE80211_VAPS_LOCK_INIT(_ic, _name)		\
	spin_lock_init(&(_ic)->ic_vapslock)
#define IEEE80211_VAPS_LOCK_DESTROY(_ic)
#define IEEE80211_VAPS_LOCK_BH(_ic)	do { 		\
	IEEE80211_VAPS_LOCK_CHECK(_ic);		\
	spin_lock_bh(&(_ic)->ic_vapslock);
#define IEEE80211_VAPS_UNLOCK_BH(_ic)			\
	IEEE80211_VAPS_LOCK_ASSERT(_ic);		\
	spin_unlock_bh(&(_ic)->ic_vapslock);		\
} while (0)
#define	IEEE80211_VAPS_LOCK_IRQ(_ic) do {					\
	unsigned long __ilockflags;					\
	IEEE80211_VAPS_LOCK_CHECK(_ic);					\
	spin_lock_irqsave(&(_ic)->ic_vapslock, __ilockflags);
#define	IEEE80211_VAPS_UNLOCK_IRQ(_ic)					\
	IEEE80211_VAPS_LOCK_ASSERT(_ic);					\
	spin_unlock_irqrestore(&(_ic)->ic_vapslock, __ilockflags);	\
} while (0)

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define IEEE80211_VAPS_LOCK_ASSERT(_ic) \
	KASSERT(spin_is_locked(&(_ic)->ic_vapslock), \
		("ieee80211com_vaps not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	IEEE80211_VAPS_LOCK_CHECK(_ic) do { \
	if (spin_is_locked(&(_ic)->ic_vapslock)) \
		printk("%s:%d - about to block on ieee80211com_vaps lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define IEEE80211_VAPS_LOCK_CHECK(_ic)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define IEEE80211_VAPS_LOCK_ASSERT(_ic)
#define IEEE80211_VAPS_LOCK_CHECK(_ic)
#endif

/*
 * Node locking definitions.
 */
#if 0

typedef spinlock_t ieee80211_node_lock_t;
#define	IEEE80211_NODE_LOCK_INIT(_ni, _name)	spin_lock_init(&(_ni)->ni_nodelock)
#define	IEEE80211_NODE_LOCK_DESTROY(_ni)
#define	IEEE80211_NODE_LOCK_IRQ(_ni)	do {	\
	unsigned long __node_lockflags;		\
	IEEE80211_NODE_LOCK_CHECK(_ni); 	\
	spin_lock_irqsave(&(_ni)->ni_nodelock, __node_lockflags);
#define	IEEE80211_NODE_UNLOCK_IRQ(_ni) \
	IEEE80211_NODE_LOCK_ASSERT(_ni); \
	spin_unlock_irqrestore(&(_ni)->ni_nodelock, __node_lockflags); \
} while (0)
#define	IEEE80211_NODE_UNLOCK_IRQ_EARLY(_ni)		\
	IEEE80211_NODE_LOCK_ASSERT(_ni); \
	spin_unlock_irqrestore(&(_ni)->ni_nodelock, __node_lockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	IEEE80211_NODE_LOCK_ASSERT(_ni) \
	KASSERT(spin_is_locked(&(_ni)->ni_nodelock), \
		("802.11 node not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	IEEE80211_NODE_LOCK_CHECK(_ni) do { \
	if (spin_is_locked(&(_ni)->ni_nodelock)) \
		printk("%s:%d - about to block on node lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	IEEE80211_NODE_LOCK_CHECK(_ni)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	IEEE80211_NODE_LOCK_ASSERT(_ni)
#define	IEEE80211_NODE_LOCK_CHECK(_ni)
#endif

#endif				/* node lock */

/*
 * Node table locking definitions.
 */
typedef spinlock_t ieee80211_node_table_lock_t;
#define	IEEE80211_NODE_TABLE_LOCK_INIT(_nt, _name)	spin_lock_init(&(_nt)->nt_nodelock)
#define	IEEE80211_NODE_TABLE_LOCK_DESTROY(_nt)
#if 0				/* We should always be contesting in the same contexts */
#define	IEEE80211_NODE_TABLE_LOCK(_nt)	spin_lock(&(_nt)->nt_nodelock)
#define	IEEE80211_NODE_TABLE_UNLOCK(_nt)	spin_unlock(&(_nt)->nt_nodelock)
#define	IEEE80211_NODE_TABLE_LOCK_BH(_nt)	spin_lock_bh(&(_nt)->nt_nodelock)
#define	IEEE80211_NODE_TABLE_UNLOCK_BH(_nt)	spin_unlock_bh(&(_nt)->nt_nodelock)
#endif
#define	IEEE80211_NODE_TABLE_LOCK_IRQ(_nt)	do {	\
	unsigned long __node_lockflags;		\
	spin_lock_irqsave(&(_nt)->nt_nodelock, __node_lockflags);
#define	IEEE80211_NODE_TABLE_UNLOCK_IRQ(_nt)		\
	spin_unlock_irqrestore(&(_nt)->nt_nodelock, __node_lockflags); \
} while (0)
#define	IEEE80211_NODE_TABLE_UNLOCK_IRQ_EARLY(_nt)		\
	spin_unlock_irqrestore(&(_nt)->nt_nodelock, __node_lockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	IEEE80211_NODE_TABLE_LOCK_ASSERT(_nt) \
	KASSERT(spin_is_locked(&(_nt)->nt_nodelock), \
		("802.11 node table not locked!"))
#else
#define	IEEE80211_NODE_TABLE_LOCK_ASSERT(_nt)
#endif

/*
 * Node table scangen locking definitions.
 */
typedef spinlock_t ieee80211_scan_lock_t;
#define	IEEE80211_SCAN_LOCK_INIT(_nt, _name) spin_lock_init(&(_nt)->nt_scanlock)
#define	IEEE80211_SCAN_LOCK_DESTROY(_nt)
#define	IEEE80211_SCAN_LOCK_IRQ(_nt)	do {	\
	unsigned long __scan_lockflags;		\
	spin_lock_irqsave(&(_nt)->nt_scanlock, __scan_lockflags);
#define	IEEE80211_SCAN_UNLOCK_IRQ(_nt)		\
	spin_unlock_irqrestore(&(_nt)->nt_scanlock, __scan_lockflags); \
} while (0)
#define	IEEE80211_SCAN_UNLOCK_IRQ_EARLY(_nt)		\
	spin_unlock_irqrestore(&(_nt)->nt_scanlock, __scan_lockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	IEEE80211_SCAN_LOCK_ASSERT(_nt) \
	KASSERT(spin_is_locked(&(_nt)->nt_scanlock), ("scangen not locked!"))
#else
#define	IEEE80211_SCAN_LOCK_ASSERT(_nt)
#endif

/*
 * 802.1x MAC ACL database locking definitions.
 */
typedef spinlock_t acl_lock_t;
#define	ACL_LOCK_INIT(_as, _name)	spin_lock_init(&(_as)->as_lock)
#define	ACL_LOCK_DESTROY(_as)
#define	ACL_LOCK_IRQ(_as)	do {	\
	unsigned long __acl_lockflags;		\
	spin_lock_irqsave(&(_as)->as_lock, __acl_lockflags);
#define	ACL_UNLOCK_IRQ(_as) \
	spin_unlock_irqrestore(&(_as)->as_lock, __acl_lockflags); \
} while (0)
#define	ACL_UNLOCK_IRQ_EARLY(_as)	do { \
	spin_unlock_irqrestore(&(_as)->as_lock, __acl_lockflags); \
} while (0)

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define	ACL_LOCK_ASSERT(_as) \
	KASSERT(spin_is_locked(&(_as)->as_lock), ("ACL not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define	ACL_LOCK_CHECK(_as) do { \
	if (spin_is_locked(&(_as)->as_lock)) \
		printk("%s:%d - about to block on ACL lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define	ACL_LOCK_CHECK(_as)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define	ACL_LOCK_ASSERT(_as)
#define	ACL_LOCK_CHECK(_as)
#endif

/*
 * Per-node power-save queue definitions.  Beware of control
 * flow with IEEE80211_NODE_SAVEQ_LOCK/IEEE80211_NODE_SAVEQ_UNLOCK.
 */
#define	IEEE80211_NODE_SAVEQ_INIT(_ni, _name) do {		\
	skb_queue_head_init(&(_ni)->ni_savedq);			\
} while (0)
#define	IEEE80211_NODE_SAVEQ_DESTROY(_ni)
#define	IEEE80211_NODE_SAVEQ_QLEN(_ni)		skb_queue_len(&(_ni)->ni_savedq)
#define	IEEE80211_NODE_SAVEQ_LOCK_IRQ(_ni) do {			\
	unsigned long __qlockflags;				\
	IEEE80211_NODE_SAVEQ_LOCK_CHECK(_ni);		\
	spin_lock_irqsave(&(_ni)->ni_savedq.lock, __qlockflags);
#define	IEEE80211_NODE_SAVEQ_UNLOCK_IRQ(_ni)    		\
	IEEE80211_NODE_SAVEQ_LOCK_ASSERT(_ni);			\
	spin_unlock_irqrestore(&(_ni)->ni_savedq.lock, __qlockflags); \
} while (0)
#define	IEEE80211_NODE_SAVEQ_UNLOCK_IRQ_EARLY(_ni)		\
	IEEE80211_NODE_SAVEQ_LOCK_ASSERT(_ni);			\
	spin_unlock_irqrestore(&(_ni)->ni_savedq.lock, __qlockflags);

#if (defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)) && defined(spin_is_locked)
#define IEEE80211_NODE_SAVEQ_LOCK_ASSERT(_ni) \
	KASSERT(spin_is_locked(&(_ni)->ni_savedq.lock), \
		("node saveq not locked!"))
#if (defined(ATH_DEBUG_SPINLOCKS))
#define IEEE80211_NODE_SAVEQ_LOCK_CHECK(_ni) do { \
	if (spin_is_locked(&(_ni)->ni_savedq.lock)) \
		printk("%s:%d - about to block on node saveq lock!\n", __func__, __LINE__); \
} while(0)
#else				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#define IEEE80211_NODE_SAVEQ_LOCK_CHECK(_ni)
#endif				/* #if (defined(ATH_DEBUG_SPINLOCKS)) */
#else
#define IEEE80211_NODE_SAVEQ_LOCK_ASSERT(_ni)
#define IEEE80211_NODE_SAVEQ_LOCK_CHECK(_ni)
#endif

/* caller MUST lock IEEE80211_NODE_SAVEQ */
#define	IEEE80211_NODE_SAVEQ_DEQUEUE(_ni, _skb, _qlen) do {	\
	_skb = __skb_dequeue(&(_ni)->ni_savedq);		\
	(_qlen) = skb_queue_len(&(_ni)->ni_savedq);		\
} while (0)
#define	_IEEE80211_NODE_SAVEQ_ENQUEUE(_ni, _skb, _qlen, _age) do {	\
	struct sk_buff *tail = skb_peek_tail(&(_ni)->ni_savedq);	\
	if (tail != NULL) {						\
		_age -= M_AGE_GET(tail);				\
		__skb_queue_after(&(_ni)->ni_savedq, tail, _skb);	\
	} else { 							\
		__skb_queue_head(&(_ni)->ni_savedq, _skb);		\
	}								\
	M_AGE_SET(_skb, _age);						\
	(_qlen) = skb_queue_len(&(_ni)->ni_savedq); 			\
} while (0)

/*
 * Transmitted frames have the following information
 * held in the sk_buff control buffer.  This is used to
 * communicate various inter-procedural state that needs
 * to be associated with the frame for the duration of
 * its existence.
 *
 * NB: sizeof(cb) == 48 and the vlan code grabs the first
 *     8 bytes so we reserve/avoid it.
 */
struct ieee80211_cb {
	u_int64_t beacon_tsf;
	struct ieee80211_node *ni;
	u_int32_t flags;
#define	M_LINK0		0x01	/* frame needs WEP encryption */
#define	M_FF		0x02	/* fast frame */
#define	M_PWR_SAV	0x04	/* bypass power save handling */
#define M_UAPSD		0x08	/* frame flagged for u-apsd handling */
#define M_RAW           0x10
#ifdef IEEE80211_DEBUG_REFCNT
	int tracked;
	void (*next_destructor)(struct sk_buff * skb);
#endif
	struct sk_buff *next;	/* fast frame sk_buf chain */
	u_int8_t auth_pkt;
};

#define	SKB_CB(_skb) 		((struct ieee80211_cb *)(_skb)->cb)

#define M_FLAG_SET(_skb, _flag) \
	(SKB_CB(_skb)->flags |= (_flag))
#define	M_FLAG_CLR(_skb, _flag) \
	(SKB_CB(_skb)->flags &= ~(_flag))
#define	M_FLAG_GET(_skb, _flag) \
	(SKB_CB(_skb)->flags & (_flag))
#define M_FLAG_KEEP_ONLY(_skb, _flag) \
	(SKB_CB(_skb)->flags &= (_flag))

#define	M_PWR_SAV_SET(skb) M_FLAG_SET((skb), M_PWR_SAV)
#define	M_PWR_SAV_CLR(skb) M_FLAG_CLR((skb), M_PWR_SAV)
#define	M_PWR_SAV_GET(skb) M_FLAG_GET((skb), M_PWR_SAV)

/*
 * Skbufs on the power save queue are tagged with an age and
 * timed out.  We reuse the hardware checksum field in the
 * mbuf packet header to store this data.
 * XXX use private cb area
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
#define skb_age csum_offset
#else
#define skb_age csum
#endif

#define	M_AGE_SET(skb,v)	(skb->skb_age = v)
#define	M_AGE_GET(skb)		(skb->skb_age)
#define	M_AGE_SUB(skb,adj)	(skb->skb_age -= adj)

struct ieee80211com;
struct ieee80211vap;

int ieee80211_load_module(const char *);

#define	le16toh(_x)	le16_to_cpu(_x)
#define	htole16(_x)	cpu_to_le16(_x)
#define	le32toh(_x)	le32_to_cpu(_x)
#define	htole32(_x)	cpu_to_le32(_x)
#define	be32toh(_x)	be32_to_cpu(_x)
#define	htobe32(_x)	cpu_to_be32(_x)

/*
 * Linux has no equivalents to malloc types so null these out.
 */
#define	MALLOC_DEFINE(type, shortdesc, longdesc)
#define	MALLOC_DECLARE(type)

/*
 * flags to malloc.
 */
#define	M_NOWAIT	0x0001	/* do not block */
#define	M_WAITOK	0x0002	/* ok to block */
#define	M_ZERO		0x0100	/* bzero the allocation */

static __inline void *ieee80211_malloc(size_t size, int flags)
{
	void *p = kmalloc(size, flags & M_NOWAIT ? GFP_ATOMIC : GFP_KERNEL);
	if (p && (flags & M_ZERO))
		memset(p, 0, size);
	return p;
}

#define	MALLOC(_ptr, cast, _size, _type, _flags) \
	((_ptr) = (cast)ieee80211_malloc(_size, _flags))
#define	FREE(addr, type)	kfree((addr))

/*
 * This unlikely to be popular but it dramatically reduces diffs.
 */
#define printf(...) printk(__VA_ARGS__)
struct ieee80211com;

#ifndef CONFIG_PRINTK
#define if_printf(net,fmt,...) do {} while(0)
#else
extern void if_printf(struct net_device *, const char *, ...);
#endif
/*
 * Queue write-arounds and support routines.
 */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_getmgtframe(_ppfrm, _pktlen) \
	ieee80211_getmgtframe_debug(_ppfrm, _pktlen, __func__, __LINE__)
extern struct sk_buff *ieee80211_getmgtframe_debug(u_int8_t **frm, u_int pktlen, const char *func, int line);
#else
extern struct sk_buff *ieee80211_getmgtframe(u_int8_t **frm, u_int pktlen);
#endif

#define	IF_ENQUEUE(_q,_skb)	skb_queue_tail(_q, _skb)
#define	IF_DEQUEUE(_q,_skb)	(_skb = skb_dequeue(_q))
#define	_IF_QLEN(_q)		skb_queue_len(_q)
#define	IF_DRAIN(_q)		skb_queue_drain(_q)
extern void skb_queue_drain(struct sk_buff_head *q);

#ifdef SINGLE_MODULE
#define _MOD_DEC_USE(_m) do {} while(0)
#define _MOD_INC_USE(_m, _err) do {} while(0)
#elif !defined(__MOD_INC_USE_COUNT)
#define	_MOD_INC_USE(_m, _err)						\
	if (!try_module_get(_m)) {					\
		printk(KERN_WARNING "%s: try_module_get failed\n",	\
			__func__); 					\
		_err;							\
	}
#define	_MOD_DEC_USE(_m)		module_put(_m)
#else
#define	_MOD_INC_USE(_m, _err)	MOD_INC_USE_COUNT
#define	_MOD_DEC_USE(_m)	MOD_DEC_USE_COUNT
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
static __inline u_int64_t get_jiffies_64(void)
{
	return (u_int64_t)jiffies;	/* XXX not right */
}
#endif

/* msecs_to_jiffies appeared in 2.6.7 and 2.4.29 */
#include <linux/delay.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && \
	 LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)) || \
	LINUX_VERSION_CODE < KERNEL_VERSION(2,4,29)

/* The following definitions and inline functions are
 * copied from the kernel src, include/linux/jiffies.h */

#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC (1000L)
#endif

#ifndef MAX_JIFFY_OFFSET
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)
#endif

static __inline unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return (j + (HZ / MSEC_PER_SEC) - 1) / (HZ / MSEC_PER_SEC);
#else
	return (j * MSEC_PER_SEC) / HZ;
#endif
}

static __inline unsigned long msecs_to_jiffies(const unsigned int m)
{
	if (m > jiffies_to_msecs(MAX_JIFFY_OFFSET))
		return MAX_JIFFY_OFFSET;
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return m * (HZ / MSEC_PER_SEC);
#else
	return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
#endif
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,7)
#include <linux/jiffies.h>
#endif

#ifndef CLONE_KERNEL
/*
 * List of flags we want to share for kernel threads,
 * if only because they are not used by them anyway.
 */
#define CLONE_KERNEL	(CLONE_FS | CLONE_FILES | CLONE_SIGHAND)
#endif

#include <linux/mm.h>
#ifndef offset_in_page
#define	offset_in_page(p) ((unsigned long) (p) & ~PAGE_MASK)
#endif

#ifndef module_put_and_exit
#define module_put_and_exit(code) do {	\
	_MOD_DEC_USE(THIS_MODULE);	\
	do_exit(code);			\
} while (0)
#endif

/*
 * Linux uses __BIG_ENDIAN and __LITTLE_ENDIAN while BSD uses _foo
 * and an explicit _BYTE_ORDER.  Sorry, BSD got there first--define
 * things in the BSD way...
 */
#undef _LITTLE_ENDIAN
#define	_LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
#undef _BIG_ENDIAN
#define	_BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
#include <asm/byteorder.h>
#if defined(__LITTLE_ENDIAN)
#define	_BYTE_ORDER	_LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN)
#define	_BYTE_ORDER	_BIG_ENDIAN
#else
#error "Please fix asm/byteorder.h"
#endif

/*
 * Deal with the sysctl handler api changing.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
#define	IEEE80211_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
	f(ctl_table *ctl, int write, struct file *filp, \
	  void __user *buffer, size_t *lenp)
#define	IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, filp, buffer, lenp)
#define IEEE80211_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
        proc_dostring(ctl, write, filp, buffer, lenp)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
#define	IEEE80211_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
 	f(ctl_table *ctl, int write, struct file *filp, \
 	  void __user *buffer, size_t *lenp, loff_t *ppos)
#define	IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
 	proc_dointvec(ctl, write, filp, buffer, lenp, ppos)
#define IEEE80211_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
        proc_dostring(ctl, write, filp, buffer, lenp, ppos)
#else				/* Linux 2.6.32+ */
#define	IEEE80211_SYSCTL_DECL(f, ctl, write, filp, buffer, lenp, ppos) \
	f(ctl_table *ctl, int write, \
	  void __user *buffer, size_t *lenp, loff_t *ppos)
#define	IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, buffer, lenp, ppos)
#define IEEE80211_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
        proc_dostring(ctl, write, buffer, lenp, ppos)
#endif

void ieee80211_virtfs_latevattach(struct ieee80211vap *);
void ieee80211_virtfs_vdetach(struct ieee80211vap *);
int ieee80211_proc_vcreate(struct ieee80211vap *, struct file_operations *, char *);
void ieee80211_proc_cleanup(struct ieee80211vap *);

#define IEEE80211_VLAN_TAG_USED 0
void ieee80211_vlan_vattach(struct ieee80211vap *);
void ieee80211_vlan_vdetach(struct ieee80211vap *);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define	free_netdev(dev)	kfree(dev)
#endif

void ieee80211_update_link_status(struct ieee80211vap *vap, int nstate, int ostate);
void ieee80211_ioctl_vattach(struct ieee80211vap *);
void ieee80211_ioctl_vdetach(struct ieee80211vap *);
struct ifreq;
int ieee80211_ioctl_create_vap(struct ieee80211com *, struct ifreq *, struct net_device *);
struct ieee80211vap *ieee80211_create_vap(struct ieee80211com *, char *, struct net_device *, int, int, struct ieee80211vap *);
extern const char *ether_sprintf(const u_int8_t *);

#endif				/* _NET80211_IEEE80211_LINUX_H_ */
