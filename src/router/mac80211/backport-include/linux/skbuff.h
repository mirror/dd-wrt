#ifndef __BACKPORT_SKBUFF_H
#define __BACKPORT_SKBUFF_H
#include_next <linux/skbuff.h>
#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(3,4,0) && \
      (RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(6,4)) && \
      !(defined(CONFIG_SUSE_KERNEL) && LINUX_VERSION_IS_GEQ(3,0,0))
#define skb_add_rx_frag(skb, i, page, off, size, truesize) \
	skb_add_rx_frag(skb, i, page, off, size)
#endif

#if LINUX_VERSION_IS_LESS(3,3,0)
#define skb_complete_wifi_ack LINUX_BACKPORT(skb_complete_wifi_ack)
void skb_complete_wifi_ack(struct sk_buff *skb, bool acked);
#endif

#if LINUX_VERSION_IS_LESS(3,3,0)
#define __pskb_copy LINUX_BACKPORT(__pskb_copy)
extern struct sk_buff *__pskb_copy(struct sk_buff *skb,
				   int headroom, gfp_t gfp_mask);
#elif LINUX_VERSION_IS_LESS(3,18,0)
#define skb_complete_wifi_ack LINUX_BACKPORT(skb_complete_wifi_ack)
void skb_complete_wifi_ack(struct sk_buff *skb, bool acked);
#endif


#if LINUX_VERSION_IS_LESS(4,14,0)
static inline void *backport_skb_put(struct sk_buff *skb, unsigned int len)
{
	return skb_put(skb, len);
}
#define skb_put LINUX_BACKPORT(skb_put)

static inline void *backport_skb_push(struct sk_buff *skb, unsigned int len)
{
	return skb_push(skb, len);
}
#define skb_push LINUX_BACKPORT(skb_push)

static inline void *backport___skb_push(struct sk_buff *skb, unsigned int len)
{
	return __skb_push(skb, len);
}
#define __skb_push LINUX_BACKPORT(__skb_push)

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
#endif

#if LINUX_VERSION_IS_LESS(4,20,0)
static inline struct sk_buff *__skb_peek(const struct sk_buff_head *list_)
{
	return list_->next;
}

#if !LINUX_VERSION_IN_RANGE(4,19,10, 4,20,0) && \
    !LINUX_VERSION_IN_RANGE(4,14,217, 4,15,0)
static inline void skb_mark_not_on_list(struct sk_buff *skb)
{
	skb->next = NULL;
}
#endif /* 4.19.10 <= x < 4.20 */
#endif

#if LINUX_VERSION_IS_LESS(4,11,0)
#define skb_mac_offset LINUX_BACKPORT(skb_mac_offset)
static inline int skb_mac_offset(const struct sk_buff *skb)
{
	return skb_mac_header(skb) - skb->data;
}
#endif

#if LINUX_VERSION_IS_LESS(5,4,0)
/**
 * skb_frag_off() - Returns the offset of a skb fragment
 * @frag: the paged fragment
 */
#define skb_frag_off LINUX_BACKPORT(skb_frag_off)
static inline unsigned int skb_frag_off(const skb_frag_t *frag)
{
	return frag->page_offset;
}

#define nf_reset_ct LINUX_BACKPORT(nf_reset_ct)
static inline void nf_reset_ct(struct sk_buff *skb)
{
	nf_reset(skb);
}
#endif

#ifndef skb_list_walk_safe
#define skb_list_walk_safe(first, skb, next_skb)				\
	for ((skb) = (first), (next_skb) = (skb) ? (skb)->next : NULL; (skb); 	\
	     (skb) = (next_skb), (next_skb) = (skb) ? (skb)->next : NULL)
#endif

#if LINUX_VERSION_IS_LESS(5,6,0) &&			\
	!LINUX_VERSION_IN_RANGE(5,4,69, 5,5,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,19,149, 4,20,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,14,200, 4,15,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,9,238, 4,10,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,4,238, 4,5,0)
/**
 *	skb_queue_len_lockless	- get queue length
 *	@list_: list to measure
 *
 *	Return the length of an &sk_buff queue.
 *	This variant can be used in lockless contexts.
 */
#define skb_queue_len_lockless LINUX_BACKPORT(skb_queue_len_lockless)
static inline __u32 skb_queue_len_lockless(const struct sk_buff_head *list_)
{
	return READ_ONCE(list_->qlen);
}
#endif /* < 5.6.0 */

#if LINUX_VERSION_IS_LESS(5,11,0)
#define skb_get_kcov_handle LINUX_BACKPORT(skb_get_kcov_handle)
static inline u64 skb_get_kcov_handle(struct sk_buff *skb)
{
	return 0;
}
#endif

#if LINUX_VERSION_IS_LESS(3,19,0)
#define skb_copy_datagram_msg LINUX_BACKPORT(skb_copy_datagram_msg)
static inline int skb_copy_datagram_msg(const struct sk_buff *from, int offset,
					struct msghdr *msg, int size)
{
	return skb_copy_datagram_iovec(from, offset, msg->msg_iov, size);
}

#define memcpy_from_msg LINUX_BACKPORT(memcpy_from_msg)
static inline int memcpy_from_msg(void *data, struct msghdr *msg, int len)
{
	return memcpy_fromiovec(data, msg->msg_iov, len);
}

/**
 *	skb_put_padto - increase size and pad an skbuff up to a minimal size
 *	@skb: buffer to pad
 *	@len: minimal length
 *
 *	Pads up a buffer to ensure the trailing bytes exist and are
 *	blanked. If the buffer already contains sufficient data it
 *	is untouched. Otherwise it is extended. Returns zero on
 *	success. The skb is freed on error.
 */
#define skb_put_padto LINUX_BACKPORT(skb_put_padto)
static inline int skb_put_padto(struct sk_buff *skb, unsigned int len)
{
	unsigned int size = skb->len;

	if (unlikely(size < len)) {
		len -= size;
		if (skb_pad(skb, len))
			return -ENOMEM;
		__skb_put(skb, len);
	}
	return 0;
}

#define skb_ensure_writable LINUX_BACKPORT(skb_ensure_writable)
int skb_ensure_writable(struct sk_buff *skb, int write_len);

#endif /* LINUX_VERSION_IS_LESS(3,19,0) */

#if LINUX_VERSION_IS_LESS(3,18,0)
#define skb_clone_sk LINUX_BACKPORT(skb_clone_sk)
struct sk_buff *skb_clone_sk(struct sk_buff *skb);
#endif

#if LINUX_VERSION_IS_LESS(4,2,0)
static inline void skb_free_frag(void *data)
{
	put_page(virt_to_head_page(data));
}

#if LINUX_VERSION_IS_LESS(3,3,0)

static inline u32 skb_get_hash_perturb(struct sk_buff *skb, u32 key)
{
	return 0;
}

#else
#include <net/flow_keys.h>
#include <linux/jhash.h>

static inline u32 skb_get_hash_perturb(struct sk_buff *skb, u32 key)
{
	struct flow_keys keys;

	skb_flow_dissect(skb, &keys);
	return jhash_3words((__force u32)keys.dst,
			    (__force u32)keys.src ^ keys.ip_proto,
			    (__force u32)keys.ports, key);
}
#endif /* LINUX_VERSION_IS_LESS(3,3,0) */
#endif /* LINUX_VERSION_IS_LESS(4,2,0) */

#if LINUX_VERSION_IS_LESS(3,7,0) && !LINUX_VERSION_IS_LESS(3,3,0)
static inline void kfree_skb_list(struct sk_buff *segs)
{
	while (segs) {
		struct sk_buff *next = segs->next;

		kfree_skb(segs);
		segs = next;
	}
}
#endif
#ifndef skb_list_walk_safe
/* Iterate through singly-linked GSO fragments of an skb. */
#define skb_list_walk_safe(first, skb, next_skb)                               \
	for ((skb) = (first), (next_skb) = (skb) ? (skb)->next : NULL; (skb);  \
	     (skb) = (next_skb), (next_skb) = (skb) ? (skb)->next : NULL)
#endif

#endif /* __BACKPORT_SKBUFF_H */
