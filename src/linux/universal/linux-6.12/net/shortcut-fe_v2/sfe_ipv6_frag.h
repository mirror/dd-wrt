/*
 * sfe_ipv6_frag.h
 *	Shortcut forwarding engine header file for IPv6 fragment forwarding.
 *
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __SFE_IPV6_FRAG_H
#define __SFE_IPV6_FRAG_H

/*
 * Maximum number of fragmend id entities that can be allocated.
 */
#define SFE_IPV6_FRAG_ID_TOTAL_HIGH_TRESH 500

/*
 * Maximum number fragments that can be queued per fragment-id.
 */
#define SFE_IPV6_FRAG_ID_QUEUE_LIMIT 2

/*
 * Fragment id entitiy timeout(30 seconds).
 */
#define SFE_IPV6_FRAG_ID_TIMEOUT_DEFAULT 3000

/*
 * Frequency of the timer to free timedout fragment id entities.
 */
#define SFE_IPV6_FRAG_ID_TIMEOUT_TIMER_FREQ 100

/*
 * IPv6 fragment id entity hash table information.
 */
#define SFE_IPV6_FRAG_HASH_SHIFT 10
#define SFE_IPV6_FRAG_HASH_SIZE (1 << SFE_IPV6_FRAG_HASH_SHIFT)
#define SFE_IPV6_FRAG_HASH_MASK (SFE_IPV6_FRAG_HASH_SIZE - 1)

/*
 * sfe_ipv6_frag_exception
 *	Per CPU exception object.
 */
struct sfe_ipv6_frag_exception {
	struct timer_list timer;	/* Timer to exception the fragments */
	struct sk_buff_head exception_queue;
			/* SKB list to queue the fragments to be exceptioned */
};

/*
 * sfe_ipv6_frag
 *	Base IPV6 Fragment handling object.
 */
struct sfe_ipv6_frag {
	struct sfe_ipv6 *si;		/* Pointer to SFE IPv6 object */
	struct timer_list timer;	/* Timer to free timed-out fragment entries */
	struct list_head lru_head;	/* LRU list of fragment entries being processed */
	struct sfe_ipv6_frag_exception __percpu *pcpu;
					/* Per CPU exception object. */
	spinlock_t lock;		/* Spin lock used to access LRU list and hash table*/
	u32 frag_id_entity_counter;	/* Number of fragment id entities in the lru_list */
	struct hlist_head frag_id_hashtable[SFE_IPV6_FRAG_HASH_SIZE];
					/* Fragment hash table */
	u32 hash_rand;			/* Random number used in hash calculation */
					/* Spin lock to protect the hash table */
	struct sk_buff_head exception_q;
					/* Queue to store skb to be exceptioned. */
	struct work_struct work;	/* Work to exception the skbs. */
	u16 high_tresh;			/* Higher tresh for the number of fragments we can allocate */
	u8 qlen_max;			/* Maximum Number of fragments that can be queued per 3 tuple */
	unsigned long timeout;		/* Timeout value in ms */
};

/*
 * sfe_ipv6_frag_id_map
 *	Key used to lookup fragment id.
 */
struct sfe_ipv6_frag_id_map {
	struct sfe_ipv6_addr src_ip;	/* Source IPv6 Address */
	struct sfe_ipv6_addr dest_ip;	/* Destination IPv6 Address */
	__be32 frag_id;			/* Fragment ID */
};

/*
 * sfe_ipv6_frag_id_element
 *	Stores every sub elements(fragments) together for a specific fragment ID.
 *
 * This is used to hold the out of order intermediate fragments in SFE till the
 * head fragment (fragment with offset 0) is received.
 */
struct sfe_ipv6_frag_id_element {
	struct sk_buff *skb;		/* SKB stored in the queue */
	struct list_head q_node;	/* Link in frag ID queue */
	struct ipv6hdr *iph;		/* Pointer to IPv6 header */
	unsigned int ihl;		/* IP header length */
	struct sfe_l2_info l2_info;	/* l2 info associated with the skb */
};

/*
 * sfe_ipv6_frag_id_state_t.
 *	State of the fragment id.
 */
typedef enum {
	SFE_IPV6_FRAG_FORWARD_NEW = 0,		/* Newly created fragment id entity. */
	SFE_IPV6_FRAG_FORWARD_BUF,		/* Queue the packets to fragment id entity. */
	SFE_IPV6_FRAG_FORWARD_EXCEPTION,	/* Exception the packets to host. */
	SFE_IPV6_FRAG_FORWARD,			/* Forward the packets. */
	SFE_IPV6_FRAG_FORWARD_DROP,		/* Drop the packets. */
	SFE_IPV6_FRAG_FORWARD_MAX,		/* Fragment state maximum. */
} sfe_ipv6_frag_id_state_t;

/*
 * sfe_ipv6_frag_id_entity.
 *	Stores fragment ID and the 5-tuple mapping.
 */
struct sfe_ipv6_frag_id_entity {
	struct sfe_ipv6_frag *sif;	/* Pointer to fragment handling object */
	struct hlist_node hnode;	/* Hash list node */
	struct list_head lru_node;	/* LRU list node */
	struct list_head q_head;	/* Fragment queue head */
	struct sfe_ipv6_addr src_ip[1];	/* Source IP address */
	struct sfe_ipv6_addr dest_ip[1];	/* Destination IP address */
	__be32 frag_id;			/* Fragment id */
	__be16 src_port;		/* Source port */
	__be16 dest_port;		/* Destination port */
	u8 protocol;			/* Protocol */
	u8 qlen;			/* Lenght of skbs queued */
	spinlock_t lock;		/* Spin lock to protect the fragment id entity */
	refcount_t refcnt;		/* Reference count to handle freeing of fragment id entity */
	unsigned long timeout;		/* Timeout value after which this fragment id entity has to be freed */
	sfe_ipv6_frag_id_state_t state;	/* State of the fragment id entity */
};

struct sfe_ipv6_frag *sfe_ipv6_frag_alloc(struct sfe_ipv6 *si);
void sfe_ipv6_frag_free(struct sfe_ipv6_frag *sif);
int sfe_ipv6_recv_udp_frag(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct ipv6hdr *iph, unsigned int ihl, bool sync_on_find, struct sfe_l2_info *l2_info, bool tun_outer, struct frag_hdr *fhdr);
void sfe_ipv6_frag_init(struct sfe_ipv6_frag *sif, struct sfe_ipv6 *si);
void sfe_ipv6_frag_exit(struct sfe_ipv6_frag *sif);
#endif /* _SFE_IPV6_FRAG_H */
