/*
 * ********************************************************************************
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **********************************************************************************
 */

#ifndef __NSS_IPSECMGR_PRIV_H
#define __NSS_IPSECMGR_PRIV_H

#include <net/ipv6.h>
#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>

#define NSS_IPSECMGR_DEBUG_LVL_ERROR 1		/**< Turn on debug for an error. */
#define NSS_IPSECMGR_DEBUG_LVL_WARN 2		/**< Turn on debug for a warning. */
#define NSS_IPSECMGR_DEBUG_LVL_INFO 3		/**< Turn on debug for information. */
#define NSS_IPSECMGR_DEBUG_LVL_TRACE 4		/**< Turn on debug for trace. */

#define nss_ipsecmgr_info_always(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#define nss_ipsecmgr_error(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsecmgr_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ipsecmgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ipsecmgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#define nss_ipsecmgr_info(s, ...) {	\
	if (NSS_IPSECMGR_DEBUG_LEVEL > NSS_IPSECMGR_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsecmgr_trace(s, ...) {	\
	if (NSS_IPSECMGR_DEBUG_LEVEL > NSS_IPSECMGR_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */
#define NSS_IPSECMGR_MAX_KEY_NAME 64 /* bytes */
#define NSS_IPSECMGR_MAX_KEY_WORDS 16 /* word */

#define NSS_IPSECMGR_MAX_KEY_BYTES (NSS_IPSECMGR_MAX_KEY_WORDS * sizeof(uint32_t))
#define NSS_IPSECMGR_MAX_KEY_BITS (NSS_IPSECMGR_MAX_KEY_WORDS * sizeof(uint32_t) * BITS_PER_BYTE)
#define NSS_IPSECMGR_CHK_POW2(x) (__builtin_constant_p(x) && !(~(x - 1) & (x >> 1)))

#define NSS_IPSECMGR_MAX_NAME (NSS_IPSECMGR_MAX_KEY_NAME + 64)

#define NSS_IPSECMGR_MAX_SA NSS_CRYPTO_MAX_IDXS /* Max SAs */
#if (~(NSS_IPSECMGR_MAX_SA - 1) & (NSS_IPSEC_MAX_SA >> 1))
#error "NSS_IPSECMGR_MAX_SA is not a power of 2"
#endif

#define NSS_IPSECMGR_MAX_FLOW 256 /* Max flows */
#if (~(NSS_IPSECMGR_MAX_FLOW - 1) & (NSS_IPSECMGR_MAX_FLOW >> 1))
#error "NSS_IPSECMGR_MAX_FLOW is not a power of 2"
#endif

#define NSS_IPSECMGR_MAX_SUBNET 16 /* Max subnets */
#if (~(NSS_IPSECMGR_MAX_SUBNET - 1) & (NSS_IPSECMGR_MAX_SUBNET >> 1))
#error "NSS_IPSECMGR_MAX_SUBNET is not a power of 2"
#endif

#define NSS_IPSECMGR_MAX_NETMASK 128 /* Max subnets */
#if (~(NSS_IPSECMGR_MAX_NETMASK - 1) & (NSS_IPSECMGR_MAX_NETMASK >> 1))
#error "NSS_IPSECMGR_MAX_NETMASK is not a power of 2"
#endif

#define NSS_IPSECMGR_NETMASK_BITMAP BITS_TO_LONGS(NSS_IPSECMGR_MAX_NETMASK)

#define NSS_IPSECMGR_GENMASK(hi, lo) ((~0 >> (31 - hi)) << lo)

#define NSS_IPSECMGR_MAX_BUF_SZ 2048
#define NSS_IPSECMGR_PROTO_NEXT_HDR_ANY 0xff

#define NSS_IPSECMGR_V6_SUBNET_BITS (sizeof(uint32_t) * 2 * BITS_PER_BYTE)

#define NSS_IPSECMGR_SA_STATS_SIZE 512
#define NSS_IPSECMGR_SUBNET_STATS_SIZE 100

#define NSS_IPSECMGR_MSG_SYNC_TIMEOUT_MSECS 2000
#define NSS_IPSECMGR_MSG_SYNC_TIMEOUT_TICKS msecs_to_jiffies(NSS_IPSECMGR_MSG_SYNC_TIMEOUT_MSECS)

#define NSS_IPSECMGR_PER_FLOW_STATS_SIZE 200
#define NSS_IPSECMGR_PER_FLOW_BUF_SIZE 500
#define NSS_IPSECMGR_PER_FLOW_BUF_SRC_IP_SIZE 100
#define NSS_IPSECMGR_PER_FLOW_BUF_DST_IP_SIZE 100
#define NSS_IPSECMGR_PER_FLOW_BUF_TYPE_SIZE 100

#define NSS_IPSECMGR_BITS_PER_WORD (sizeof(uint32_t) * BITS_PER_BYTE)
#define NSS_IPSECMGR_BITS2WORD(p) ((p) / NSS_IPSECMGR_BITS_PER_WORD)

#define NSS_IPSECMGR_DEFAULT_TUN_NAME "ipsecdummy"

#define NSS_IPSECMGR_ESP_TRAIL_SZ 2 /* esp trailer size */
#define NSS_IPSECMGR_ESP_PAD_SZ 14 /* maximum amount of padding */

struct nss_ipsecmgr_ref;
struct nss_ipsecmgr_key;
struct nss_ipsecmgr_priv;
/*
 * IPsec manager key length for
 *  - SA entries
 *  - Flow entries
 *  - Subnet entries
 */
enum nss_ipsecmgr_key_len {
	NSS_IPSECMGR_KEY_LEN_NONE = 0,
	NSS_IPSECMGR_KEY_LEN_IPV4_SA = 4,		/* 16 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV4_SUBNET = 2,		/* 8 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV4_ENCAP_FLOW = 3,	/* 12 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV4_DECAP_FLOW = 4,	/* 16 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV6_SA = 10,		/* 40 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV6_SUBNET = 5,		/* 20 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV6_ENCAP_FLOW = 9,	/* 36 bytes */
	NSS_IPSECMGR_KEY_LEN_IPV6_DECAP_FLOW = 10,	/* 40 bytes */
	NSS_IPSECMGR_KEY_LEN_MAX = NSS_IPSECMGR_MAX_KEY_WORDS
};

/*
 * IPsec manager key map for header fields to key elements
 */
enum nss_ipsecmgr_key_pos {
	NSS_IPSECMGR_KEY_POS_IP_VER = 0,		/* IP version, bits[0:7] */
	NSS_IPSECMGR_KEY_POS_IP_PROTO = 8,		/* IPv4 Proto, bits[8:15] */
	NSS_IPSECMGR_KEY_POS_IPV4_DST = 32,		/* IPv4 DST, bits[32:63] */
	NSS_IPSECMGR_KEY_POS_IPV4_SRC = 64,		/* IPv4 SIP, bits[64:95] */
	NSS_IPSECMGR_KEY_POS_IPV4_ESP_SPI = 96,		/* IPv4 ESP SPI, bits[96:127] */
	NSS_IPSECMGR_KEY_POS_IPV6_DST = 32,		/* IPv6 DST, bits[32:159] */
	NSS_IPSECMGR_KEY_POS_IPV6_SRC = 160,	 	/* IPv6 SIP, bits[160:287] */
	NSS_IPSECMGR_KEY_POS_IPV6_ESP_SPI = 288,	/* IPv6 ESP SPI, bits[288:319] */


};

/*
 * bits to mask within a key_word data, min - 0 & max - 31
 */
enum nss_ipsecmgr_key_mask {
	NSS_IPSECMGR_KEY_MASK_IP_VER = NSS_IPSECMGR_GENMASK(7, 0),	/* IP version, #bits - 8 */
	NSS_IPSECMGR_KEY_MASK_IP_PROTO = NSS_IPSECMGR_GENMASK(15, 8),	/* IP protocol, #bits - 8 */
	NSS_IPSECMGR_KEY_MASK_IPV4_DST = NSS_IPSECMGR_GENMASK(31, 0),	/* IPv4 dst, #bits - 32 */
	NSS_IPSECMGR_KEY_MASK_IPV4_SRC = NSS_IPSECMGR_GENMASK(31, 0),	/* IPv4 src, #bits - 32 */
	NSS_IPSECMGR_KEY_MASK_ESP_SPI = NSS_IPSECMGR_GENMASK(31, 0),	/* ESP spi #bits - 32 */
};

struct nss_ipsecmgr_ref;
struct nss_ipsecmgr_key;
struct nss_ipsecmgr_priv;

typedef void (*nss_ipsecmgr_ref_update_t)(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref, struct nss_ipsec_msg *nim);
typedef void (*nss_ipsecmgr_ref_free_t)(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref);
typedef uint32_t (*nss_ipsecmgr_ref_overhead_t)(struct nss_ipsecmgr_ref *ref);

/*
 * Key byte stream for lookup
 */
struct nss_ipsecmgr_key {
	uint32_t data[NSS_IPSECMGR_MAX_KEY_WORDS];	/* Value N-bits */
	uint32_t mask[NSS_IPSECMGR_MAX_KEY_WORDS];	/* Mask N-bits */
	uint32_t len;					/* Length of the key */
};

/*
 * IPsec manager reference object
 */
struct nss_ipsecmgr_ref {
	struct list_head head;			/* parent "ref" */
	struct list_head node;			/* child "ref" */

	uint32_t id;				/* identifier */
	struct nss_ipsecmgr_ref *parent;	/* reference to parent */

	nss_ipsecmgr_ref_update_t update;	/* update function */
	nss_ipsecmgr_ref_free_t free;		/* free function */
	nss_ipsecmgr_ref_overhead_t overhead;	/* free function */
};

/*
 * IPsec manager packets stats per SA
 */
struct nss_ipsecmgr_sa_pkt_stats {
	uint64_t count;
	uint64_t bytes;

	uint64_t seq_num;
	uint64_t window_max;
	uint32_t window_size;

	uint32_t no_headroom;
	uint32_t no_tailroom;
	uint32_t no_buf;
	uint32_t fail_queue;
	uint32_t fail_hash;
	uint32_t fail_replay;
	uint32_t fail_hash_cont;		/* Continous fail hash count */
};

/*
 * IPsec manager node stats
 */
struct nss_ipsecmgr_node_stats {
	uint64_t enqueued;		/* packets enqueued to the node */
	uint64_t completed;		/* packets processed by the node */
	uint64_t linearized;		/* linearized the packet */
	uint64_t exceptioned;		/* packets exception from NSS */
	uint64_t fail_enqueue;		/* packets failed to enqueue */
};

/*
 * IPsec manager SA entry
 */
struct nss_ipsecmgr_sa_entry {
	struct list_head node;			/* list instance */

	struct nss_ipsecmgr_ref ref;		/* ref instance */
	struct nss_ipsecmgr_key key;		/* key instance */

	uint32_t ifnum;				/* SA interface */
	uint32_t sa_overhead;			/* max header + trailer added for SA */
	bool esn_enabled;			/* Is ESN enabled */

	struct nss_ipsecmgr_sa_pkt_stats pkts;	/* packets processed per SA */
	struct nss_ipsecmgr_priv *priv;		/* ipsecmgr private reference */
	struct nss_ipsec_msg nim;		/* ipsec message */
	struct nss_ipsecmgr_sa sa_info;		/* SA information */

	uint32_t fail_hash_thresh;		/* continous hash fail
						   threshold */
};

/*
 * IPsec manager SA entry table
 */
struct nss_ipsecmgr_sa_db {
	atomic_t num_entries;			/* active entries in the entity */

	struct list_head entries[NSS_IPSECMGR_MAX_SA];
};

/*
 * IPsec manager subnet entry
 */
struct nss_ipsecmgr_subnet_entry {
	struct list_head node;			/* list node */
	struct nss_ipsecmgr_ref ref;		/* reference node */
	struct nss_ipsecmgr_key key;		/* key */

	struct nss_ipsec_msg nim;		/* IPsec message */
	struct nss_ipsecmgr_priv *priv;		/* ipsecmgr private reference */
};

/*
 * IPsec netmask entry
 */
struct nss_ipsecmgr_netmask_entry {
	uint32_t mask_bits;					/* no. of bits in netmask */
	uint32_t count;						/* no. of subnets entries */
	struct list_head subnets[NSS_IPSECMGR_MAX_SUBNET];	/* subnet database */
};

/*
 * IPsec netmask database
 *
 * Note: this database is searched using bit positions present in bitmap
 * where each bit in bitmap table represents a entry in the mask entry
 * table
 */
struct nss_ipsecmgr_netmask_db {
	atomic_t num_entries;							/* active entries in the entity */
	unsigned long bitmap[NSS_IPSECMGR_NETMASK_BITMAP];			/* netmask bitmap */
	struct nss_ipsecmgr_netmask_entry *entries[NSS_IPSECMGR_MAX_NETMASK];	/* netmask entry database */
	struct nss_ipsecmgr_netmask_entry *default_entry;			/* default netmask */
};

/*
 * IPsec manager flow data
 */
struct nss_ipsecmgr_flow_data {
	uint32_t pkts_processed;	/* packets processed for this flow */
	uint32_t sa_overhead;		/* max header + trailer added for SA */
};

/*
 * IPsec manager flow entry
 */
struct nss_ipsecmgr_flow_entry {
	struct list_head node;			/* list object */
	struct nss_ipsecmgr_ref ref;		/* reference object */
	struct nss_ipsecmgr_key key;		/* key object */

	struct nss_ipsecmgr_priv *priv;		/* ipsecmgr private reference */
	struct nss_ipsec_msg nim;		/* IPsec message */
	struct nss_ipsecmgr_flow_data data;	/* Flow data */
};

/*
 * Flow retry
 */
struct nss_ipsecmgr_flow_retry {
	struct list_head node;			/* retry list node */
	struct nss_ipsecmgr_key key;		/* flow key for lookup */
	uint32_t ref_id;			/* refernce object id */
};

/*
 * IPsec manager flow database
 */
struct nss_ipsecmgr_flow_db {
	atomic_t num_entries;					/* active entries in the entity */
	struct list_head entries[NSS_IPSECMGR_MAX_FLOW];	/* flow database */
};

/*
 * IPsec manager callback database
 */
struct nss_ipsecmgr_callback_db {
	atomic_t num_entries;			/* number of callback(s) registered */
	struct list_head entries;		/* head of the first callback */
};

/*
 * IPsec manager callback entry
 */
struct nss_ipsecmgr_callback_entry {
	struct list_head node;			/* list node */
	int dev_index;				/* device bound for callback */
	void *app_data;				/* callback context */
	nss_ipsecmgr_data_cb_t data;		/* data callback function */
	nss_ipsecmgr_event_cb_t event;		/* event callback function */
};

/*
 * IPsec manager private context
 */
struct nss_ipsecmgr_priv {
	struct net_device *dev;			/* back pointer to tunnel device */
	struct nss_ipsecmgr_callback_entry cb;	/* callback entry instance */
	atomic_t outer_dst_mtu;			/* PMTU of outer tunnel DST */
	struct rtnl_link_stats64 stats;		/* stats of IPsec tunnel */
};

/*
 * IPsec manager drv instance
 */
struct nss_ipsecmgr_drv {
	struct dentry *dentry;			/* Debugfs entry per ipsecmgr module. */
	struct dentry *stats_dentry;		/* Debugfs entry per stats dir */
	struct net_device *ndev;		/* IPsec dummy net device */

	rwlock_t lock;				/* lock for all DB operations */

	struct nss_ipsecmgr_sa_db sa_db;	/* SA database */
	struct nss_ipsecmgr_netmask_db net_db;	/* Subnet mask database */
	struct nss_ipsecmgr_flow_db flow_db;	/* flow database */
	struct nss_ipsecmgr_callback_db cb_db;	/* callback database */
	struct completion complete;		/* completion for flow stats nss msg */

	int encap_ifnum;			/* NSS encap interface */
	int decap_ifnum;			/* NSS decap interface */
	int data_ifnum;			/* NSS data interface */

	struct nss_ctx_instance *nss_ctx;	/* NSS context */

	struct nss_ipsec_msg resp_nim;		/* nim for per_flow stats */
	struct semaphore sem;			/* per flow semaphore lock */
	atomic_t seq_num;			/* per flow seq no */

	struct nss_ipsecmgr_node_stats enc_stats;	/* Encap node stats */
	struct nss_ipsecmgr_node_stats dec_stats;	/* Decap node stats */
};

/*
 * nss_ipsecmgr_ref_is_updated()
 * 	return true if reference objects id is updated
 */
static inline bool nss_ipsecmgr_ref_is_updated(struct nss_ipsecmgr_ref *ref)
{
	return ref->id > 1;
}

/*
 * nss_ipsecmgr_ref_get_id()
 * 	return the reference object's id
 */
static inline uint32_t nss_ipsecmgr_ref_get_id(struct nss_ipsecmgr_ref *ref)
{
	return ref->id;
}

/*
 * nss_ipsecmgr_ref_is_empty()
 * 	check if the SA has any reference
 */
static inline bool nss_ipsecmgr_ref_is_empty(struct nss_ipsecmgr_ref *ref)
{
	return list_empty(&ref->head);
}

/*
 * nss_ipsecmgr_key_reset()
 * 	Reset the IPsec key
 */
static inline void nss_ipsecmgr_key_reset(struct nss_ipsecmgr_key *key)
{
	memset(key, 0, sizeof(struct nss_ipsecmgr_key));
}

/*
 * nss_ipsecmgr_key_read_8()
 * 	read value & mask from the specified position
 */
static inline uint8_t nss_ipsecmgr_key_read_8(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint32_t data;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	data = key->data[idx];

	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:7] */
		return (uint8_t)data;
	case 8: /* bits[8:15] */
		return (uint8_t)(data >> 8);
	case 16: /* bits[16:23] */
		return (uint8_t)(data >> 16);
	case 24: /* bits[24:31] */
		return (uint8_t)(data >> 24);
	default:
		return 0;
	}
}

/*
 * nss_ipsecmgr_key_write_8()
 * 	write 8-bit value from the specified position using mask
 */
static inline void nss_ipsecmgr_key_write_8(struct nss_ipsecmgr_key *key, uint8_t v, enum nss_ipsecmgr_key_pos p)
{
	uint32_t *data, *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	data = &key->data[idx];
	mask = &key->mask[idx];

	/*
	 * clear data with mask, update data & save mask
	 */
	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:7] */
		*data &= ~NSS_IPSECMGR_GENMASK(7, 0);
		*data |= v;
		*mask |= NSS_IPSECMGR_GENMASK(7, 0);
		break;
	case 8: /* bits[8:15] */
		*data &= ~NSS_IPSECMGR_GENMASK(15, 8);
		*data |= (v << 8);
		*mask |= NSS_IPSECMGR_GENMASK(15, 8);
		break;
	case 16: /* bits[16:23] */
		*data &= ~NSS_IPSECMGR_GENMASK(23, 16);
		*data |= (v << 16);
		*mask |= NSS_IPSECMGR_GENMASK(23, 16);
		break;
	case 24: /* bits[24:31] */
		*data &= ~NSS_IPSECMGR_GENMASK(31, 24);
		*data |= (v << 24);
		*mask |= NSS_IPSECMGR_GENMASK(31, 24);
		break;
	default:
		*data = 0;
		*mask = 0;
		break;
	}
}

/*
 * nss_ipsecmgr_key_clear_8()
 * 	clear 8-bit mask from the specified position.
 */
static inline void nss_ipsecmgr_key_clear_8(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint32_t *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	mask = &key->mask[idx];

	/*
	 * clear data with mask, update data & save mask
	 */
	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:7] */
		*mask &= ~NSS_IPSECMGR_GENMASK(7, 0);
		break;
	case 8: /* bits[8:15] */
		*mask &= ~NSS_IPSECMGR_GENMASK(15, 8);
		break;
	case 16: /* bits[16:23] */
		*mask &= ~NSS_IPSECMGR_GENMASK(23, 16);
		break;
	case 24: /* bits[24:31] */
		*mask &= ~NSS_IPSECMGR_GENMASK(31, 24);
		break;
	default:
		*mask = 0;
		break;
	}
}

/*
 * nss_ipsecmgr_key_read_16()
 * 	read value & mask from the specified position using mask
 */
static inline uint16_t nss_ipsecmgr_key_read_16(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint32_t data;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	data = key->data[idx];

	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:7] */
		return (uint16_t)data;
	case 16: /* bits[16:23] */
		return (uint16_t)(data >> 16);
	default:
		return 0;
	}
}

/*
 * nss_ipsecmgr_key_write_16()
 * 	write 16-bit value from the specified position using mask
 */
static inline void nss_ipsecmgr_key_write_16(struct nss_ipsecmgr_key *key, uint16_t v, enum nss_ipsecmgr_key_pos p)
{
	uint32_t *data, *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	data = &key->data[idx];
	mask = &key->mask[idx];

	/*
	 * clear data with mask, update data & save mask
	 */
	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:15] */
		*data &= ~NSS_IPSECMGR_GENMASK(15, 0);
		*data |= v;
		*mask |= NSS_IPSECMGR_GENMASK(15, 0);
		break;
	case 16: /* bits[16:31] */
		*data &= ~NSS_IPSECMGR_GENMASK(31, 16);
		*data |= (v << 16);
		*mask |= NSS_IPSECMGR_GENMASK(31, 16);
		break;
	default:
		*data = 0;
		*mask = 0;
		break;
	}
}

/*
 * nss_ipsecmgr_key_clear_16()
 * 	clear 16-bit mask from the specified position
 */
static inline void nss_ipsecmgr_key_clear_16(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint32_t *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	mask = &key->mask[idx];

	/*
	 * clear data with mask, update data & save mask
	 */
	switch (p % NSS_IPSECMGR_BITS_PER_WORD) {
	case 0: /* bits[0:15] */
		*mask &= ~NSS_IPSECMGR_GENMASK(15, 0);
		break;
	case 16: /* bits[16:31] */
		*mask &= ~NSS_IPSECMGR_GENMASK(31, 16);
		break;
	default:
		*mask = 0;
		break;
	}
}

/*
 * nss_ipsecmgr_key_read_32()
 * 	read value from the specified position using mask
 */
static inline uint32_t nss_ipsecmgr_key_read_32(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	return key->data[idx];
}

/*
 * nss_ipsecmgr_key_write_32()
 * 	write 32-bit value from the specified position using mask
 */
static inline void nss_ipsecmgr_key_write_32(struct nss_ipsecmgr_key *key, uint32_t v, enum nss_ipsecmgr_key_pos p)
{
	uint32_t *data, *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	data = &key->data[idx];
	mask = &key->mask[idx];

	/*
	 * save data & mask, bits[0:31]
	 */
	*data = v;
	*mask = NSS_IPSECMGR_GENMASK(31, 0);
}

/*
 * nss_ipsecmgr_key_clear_32()
 * 	clear 32-bit mask from the specified position
 */
static inline void nss_ipsecmgr_key_clear_32(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	key->mask[idx] = 0;
}

/*
 * nss_ipsecmgr_key_clear_64()
 * 	clear 64-bit mask from the specified position
 */
static inline void nss_ipsecmgr_key_clear_64(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	key->mask[idx] = 0;
	key->mask[idx + 1] = 0;
}

/*
 * nss_ipsecmgr_key_clear_128()
 * 	clear 128-bit mask from the specified position
 */
static inline void nss_ipsecmgr_key_clear_128(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;

	memset(&key->mask[idx], 0, sizeof(uint32_t) * 4);
}

/*
 * nss_ipsecmgr_key_read_n()
 * 	read n nnumber of values from the specified position using mask
 */
static inline void nss_ipsecmgr_key_read(struct nss_ipsecmgr_key *key, uint32_t *v, uint32_t *m, enum nss_ipsecmgr_key_pos p, uint32_t n)
{
	uint32_t *k_data, *k_mask;
	uint16_t idx;
	int delta;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	delta = NSS_IPSECMGR_MAX_KEY_WORDS - idx;

	n = n > delta ? delta : n;

	k_data = &key->data[idx];
	k_mask = &key->mask[idx];

	for (; n--;) {
		*v++ = *k_data++;
		*m++ = *k_mask++;
	}
}

/*
 * nss_ipsecmgr_key_write()
 * 	write value from the specified position using mask
 */
static inline void nss_ipsecmgr_key_write(struct nss_ipsecmgr_key *key, uint32_t *v, uint32_t *m, enum nss_ipsecmgr_key_pos p, uint32_t n)
{
	uint32_t *k_data, *k_mask;
	uint16_t idx;
	int delta;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	delta = NSS_IPSECMGR_MAX_KEY_WORDS - idx;

	n = n > delta ? delta : n;

	k_data = &key->data[idx];
	k_mask = &key->mask[idx];

	for (; n--;) {
		*k_data++ = *v++;
		*k_mask++ = *m++;
	}
}

/*
 * nss_ipsecmgr_write_key_mask()
 * 	write a mask starting from position
 */
static inline void nss_ipsecmgr_key_write_mask(struct nss_ipsecmgr_key *key, uint32_t m, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	key->mask[idx] = m;
}

/*
 * nss_ipsecmgr_read_key_mask32()
 * 	read a 32 bit mask starting from position
 */
static inline uint32_t nss_ipsecmgr_key_read_mask32(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;

	return key->mask[idx];
}

/*
 * nss_ipsecmgr_read_key_mask64()
 * 	read a 64 bit mask starting from position
 */
static inline uint64_t nss_ipsecmgr_key_read_mask64(struct nss_ipsecmgr_key *key, enum nss_ipsecmgr_key_pos p)
{
	uint64_t *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;

	mask = (uint64_t *)&key->mask[idx];

	return *mask;
}

/*
 * nss_ipsecmgr_key_lshift_mask()
 * 	left shift mask by an amount 's'
 */
static inline void nss_ipsecmgr_key_lshift_mask(struct nss_ipsecmgr_key *key, uint32_t s, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;
	key->mask[idx] <<= s;
}

/*
 * nss_ipsecmgr_key_lshift_mask64()
 * 	left shift mask by an amount 's'
 */
static inline void nss_ipsecmgr_key_lshift_mask64(struct nss_ipsecmgr_key *key, uint32_t s, enum nss_ipsecmgr_key_pos p)
{
	uint64_t *mask;
	uint16_t idx;

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;

	mask = (uint64_t *)&key->mask[idx];
	*mask <<= s;
}

/*
 * nss_ipsecmgr_key_lshift_mask128()
 * 	left shift mask by an amount 's'
 */
static inline void nss_ipsecmgr_key_lshift_mask128(struct nss_ipsecmgr_key *key, uint32_t s, enum nss_ipsecmgr_key_pos p)
{
	uint16_t idx;
	uint32_t mask[4];

	idx = NSS_IPSECMGR_BITS2WORD(p) % NSS_IPSECMGR_MAX_KEY_WORDS;

	memcpy(mask, &key->mask[idx], sizeof(uint32_t) * 4);

	bitmap_shift_left((unsigned long *)&key->mask[idx], (unsigned long *)mask, s, 128);
}

/*
 * nss_ipsecmgr_key_cmp_data()
 * 	compare source key with destination key
 *
 * Note: this applies the source mask before comparison
 */
static inline bool nss_ipsecmgr_key_cmp(struct nss_ipsecmgr_key *s_key, struct nss_ipsecmgr_key *d_key)
{
	uint32_t *mask = s_key->mask;
	uint32_t *src = s_key->data;
	uint32_t *dst = d_key->data;
	int len = s_key->len;
	uint32_t val_0;
	uint32_t val_1;
	bool status;

	for (status = 0; !status && len; len--, mask++, src++, dst++) {
		/*
		 * the data for comparison should be used after
		 * masking the bits corresponding to that word
		 */
		val_0 = *src & *mask;
		val_1 = *dst & *mask;

		status |= val_0 ^ val_1;
	}

	return status == 0;
}

/*
 * nss_ipsecmgr_key_data2idx()
 * 	convert word stream to index based on table size
 */
static inline uint32_t nss_ipsecmgr_key_data2idx(struct nss_ipsecmgr_key *key, const uint32_t table_sz)
{
	uint32_t *data = key->data;
	uint32_t *mask = key->mask;
	uint32_t len = key->len;
	uint32_t idx;
	uint32_t val;

	/*
	 * bug on if table_sz is not
	 * - a constant
	 * - and a power of 2
	 */
	BUG_ON(!NSS_IPSECMGR_CHK_POW2(table_sz));

	for (idx = 0; len; len--, mask++, data++) {
		val = *data & *mask;
		idx ^= val;
	}

	return idx & (table_sz - 1);
}

/*
 * nss_ipsecmgr_init_sa_db()
 * 	initialize the SA database
 */
static inline void nss_ipsecmgr_init_sa_db(struct nss_ipsecmgr_sa_db *sa_db)
{
	struct list_head *head;
	int i;

	/*
	 * initialize the SA database
	 */
	head = sa_db->entries;
	for (i = 0; i < NSS_IPSECMGR_MAX_SA; i++, head++) {
		INIT_LIST_HEAD(head);
	}

	atomic_set(&sa_db->num_entries, 0);
}

/*
 * nss_ipsecmgr_init_netmask_db()
 * 	initialize the netmask database
 */
static inline void nss_ipsecmgr_init_netmask_db(struct nss_ipsecmgr_netmask_db *net_db)
{
	memset(net_db, 0, sizeof(struct nss_ipsecmgr_netmask_db));

	atomic_set(&net_db->num_entries, 0);
}

/*
 * nss_ipsecmgr_init_subnet_db()
 * 	initialize the subnet database
 */
static inline void nss_ipsecmgr_init_subnet_db(struct nss_ipsecmgr_netmask_entry *netmask)
{
	struct list_head *head;
	int i;

	/*
	 * initialize the subnet database
	 */
	head = netmask->subnets;
	for (i = 0; i < NSS_IPSECMGR_MAX_SUBNET; i++, head++) {
		INIT_LIST_HEAD(head);
	}
}

/*
 * nss_ipsecmgr_init_flow_db()
 *	Initialize the flow databases
 */
static inline void nss_ipsecmgr_init_flow_db(struct nss_ipsecmgr_flow_db *flow_db)
{
	struct list_head *head;
	int i;

	/*
	 * initialize the flow database
	 */
	head = flow_db->entries;
	for (i = 0; i < NSS_IPSECMGR_MAX_FLOW; i++, head++) {
		INIT_LIST_HEAD(head);
	}

	atomic_set(&flow_db->num_entries, 0);
}

/*
 * nss_ipsecmgr_init_callback_db()
 * 	initialize the callback database
 */
static inline void nss_ipsecmgr_init_callback_db(struct nss_ipsecmgr_callback_db *cb_db)
{
	INIT_LIST_HEAD(&cb_db->entries);
	atomic_set(&cb_db->num_entries, 0);
}

/*
 * nss_ipsecmgr_v4_hdr2tuple()
 * 	convert v4_hdr to message tuple
 */
static inline void nss_ipsecmgr_v4_hdr2tuple(struct iphdr *iph, struct nss_ipsec_tuple *tuple)
{
	tuple->dst_addr[0] = ntohl(iph->daddr);
	tuple->src_addr[0] = ntohl(iph->saddr);
	tuple->proto_next_hdr = iph->protocol;
	tuple->ip_ver = IPVERSION;
}

/*
 * nss_ipsecmgr_v6addr_swap()
 * 	Swap the words in the array.
 */
static uint32_t *nss_ipsecmgr_v6addr_swap(uint32_t *src, uint32_t *dst)
{
	uint32_t temp[4];

	if (src == dst) {
		memcpy(temp, src, sizeof(temp));
		src = temp;
	}

	dst[3] = src[0];
	dst[2] = src[1];
	dst[1] = src[2];
	dst[0] = src[3];

	return dst;
}

/*
 * nss_ipsecmgr_v6addr_ntoh()
 * 	convert the v6 address form network to host order.
 */
static inline uint32_t *nss_ipsecmgr_v6addr_ntoh(uint32_t *src, uint32_t *dst)
{
	uint32_t *dst_addr = nss_ipsecmgr_v6addr_swap(src, dst);

	dst_addr[0] = ntohl(dst_addr[0]);
	dst_addr[1] = ntohl(dst_addr[1]);
	dst_addr[2] = ntohl(dst_addr[2]);
	dst_addr[3] = ntohl(dst_addr[3]);

	return dst_addr;
}

/*
 * nss_ipsecmgr_v6addr_hton()
 * 	convert the v6 address to host to network order.
 */
static inline uint32_t *nss_ipsecmgr_v6addr_hton(uint32_t *src, uint32_t *dst)
{
	uint32_t *dst_addr = nss_ipsecmgr_v6addr_swap(src, dst);

	dst_addr[0] = htonl(dst_addr[0]);
	dst_addr[1] = htonl(dst_addr[1]);
	dst_addr[2] = htonl(dst_addr[2]);
	dst_addr[3] = htonl(dst_addr[3]);

	return dst_addr;
}

/*
 * nss_ipsecmgr_copy_nim()
 * 	copy nim from source to destination, also swap the v6 addresses if any
 */
static inline void nss_ipsecmgr_copy_nim(struct nss_ipsec_msg *src_nim, struct nss_ipsec_msg *dst_nim)
{
	struct nss_ipsec_rule *src_rule = &src_nim->msg.rule;
	struct nss_ipsec_rule *dst_rule = &dst_nim->msg.rule;

	memcpy(dst_nim, src_nim, sizeof(struct nss_ipsec_msg));

	if (src_nim->tuple.ip_ver == 6) {
		nss_ipsecmgr_v6addr_swap(src_nim->tuple.dst_addr, dst_nim->tuple.dst_addr);
		nss_ipsecmgr_v6addr_swap(src_nim->tuple.src_addr, dst_nim->tuple.src_addr);
	}

	if (src_rule->oip.ip_ver == 6) {
		nss_ipsecmgr_v6addr_swap(src_rule->oip.dst_addr, dst_rule->oip.dst_addr);
		nss_ipsecmgr_v6addr_swap(src_rule->oip.src_addr, dst_rule->oip.src_addr);
	}
}

/*
 * nss_ipsecmgr_v6_hdr2tuple()
 * 	convert v6_hdr to message tuple
 */
static inline void nss_ipsecmgr_v6_hdr2tuple(struct ipv6hdr *iph, struct nss_ipsec_tuple *tuple)
{
	uint8_t nexthdr = iph->nexthdr;
	struct frag_hdr *frag;

	/*
	 * NSS IPsec module will not accelerate the flows which has ipv6 optional
	 * headers after frag_hdr. Those flows will be descarded in ipsecmgr.
	 * The NSS only accelerates when IP next header is L4 or fragment next header is L4.
	 */
	if (nexthdr == IPPROTO_FRAGMENT) {
		frag = (struct frag_hdr *)((uint8_t *)iph + sizeof(struct ipv6hdr));
		nexthdr = frag->nexthdr;
	}

	nss_ipsecmgr_v6addr_ntoh(iph->daddr.s6_addr32, tuple->dst_addr);
	nss_ipsecmgr_v6addr_ntoh(iph->saddr.s6_addr32, tuple->src_addr);

	tuple->proto_next_hdr = nexthdr;
	tuple->ip_ver = 6;
}

/*
 * nss_ipsecmgr_get_ipv4_addr()
 * 	Return ipv4 part of the address.
 */
static inline uint32_t nss_ipsecmgr_get_v4addr(uint32_t *addr)
{
	return addr[0];
}

/*
 * nss_ipsecmgr_verify_v4_subnet()
 * 	verify if v4 subnet mask is not empty when v4 subnet is empty.
 */
static inline bool nss_ipsecmgr_verify_v4_subnet(struct nss_ipsecmgr_encap_v4_subnet *v4_subnet)
{
	return !v4_subnet->dst_subnet && v4_subnet->dst_mask;
}

/*
 * nss_ipsecmgr_verify_v6_subnet()
 * 	verify if v6 subnet mask is not empty when v6 subnet is empty.
 */
static inline bool nss_ipsecmgr_verify_v6_subnet(struct nss_ipsecmgr_encap_v6_subnet *v6_subnet)
{
	bool subnet, mask;

	subnet = bitmap_empty((unsigned long *)v6_subnet->dst_subnet, NSS_IPSECMGR_V6_SUBNET_BITS);
	mask = bitmap_empty((unsigned long *)v6_subnet->dst_mask, NSS_IPSECMGR_V6_SUBNET_BITS);

	return subnet && !mask;
}

/*
 * Reference Object API(s)
 */
void nss_ipsecmgr_ref_init(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_update_t update, nss_ipsecmgr_ref_free_t free);
void nss_ipsecmgr_ref_add(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent);
void nss_ipsecmgr_ref_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref);
void nss_ipsecmgr_ref_update(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref, struct nss_ipsec_msg *nim);
bool nss_ipsecmgr_ref_is_child(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent);
void nss_ipsecmgr_ref_set_overhead(struct nss_ipsecmgr_ref *ref,
		nss_ipsecmgr_ref_overhead_t overhead);
uint32_t nss_ipsecmgr_ref_overhead(struct nss_ipsecmgr_ref *ref);

/*
 * Encap flow API(s)
 */
void nss_ipsecmgr_copy_encap_v4_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_encap_v4_tuple *flow);
void nss_ipsecmgr_copy_encap_v6_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_encap_v6_tuple *flow);
void nss_ipsecmgr_encap_v4_flow2key(struct nss_ipsecmgr_encap_v4_tuple *flow, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_encap_v6_flow2key(struct nss_ipsecmgr_encap_v6_tuple *flow, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_encap_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_encap_flow_init(struct nss_ipsec_msg *nim, enum nss_ipsec_msg_type type, struct nss_ipsecmgr_priv *priv);

/*
 * Decap flow API(s)
 */
void nss_ipsecmgr_copy_decap_v4_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v4 *flow);
void nss_ipsecmgr_copy_decap_v6_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v6 *flow);
void nss_ipsecmgr_decap_v4_flow2key(struct nss_ipsecmgr_sa_v4 *flow, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_decap_v6_flow2key(struct nss_ipsecmgr_sa_v6 *flow, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_decap_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_decap_flow_init(struct nss_ipsec_msg *nim, enum nss_ipsec_msg_type type, struct nss_ipsecmgr_priv *priv);

/*
 * flow alloc/lookup API(s)
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_flow_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
struct nss_ipsecmgr_ref *nss_ipsecmgr_flow_lookup(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
bool nss_ipsecmgr_flow_offload(struct nss_ipsecmgr_priv *priv,
		struct sk_buff *skb, struct nss_ipsecmgr_flow_data *data);
bool nss_ipsecmgr_flow_process_pmtu(struct nss_ipsecmgr_priv *priv,
		struct sk_buff *skb, struct nss_ipsecmgr_flow_data *data);

/*
 * Subnet API(s)
 */
void nss_ipsecmgr_copy_subnet(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_ref *subnet_ref);
void nss_ipsecmgr_v4_subnet2key(struct nss_ipsecmgr_encap_v4_subnet *subnet, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_v6_subnet2key(struct nss_ipsecmgr_encap_v6_subnet *subnet, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_v4_subnet_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_v6_subnet_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key);

/*
 * Subnet alloc/lookup API(s)
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_subnet_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
struct nss_ipsecmgr_ref *nss_ipsecmgr_subnet_lookup(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
struct nss_ipsecmgr_ref *nss_ipsecmgr_v4_subnet_match(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *tuple);
struct nss_ipsecmgr_ref *nss_ipsecmgr_v6_subnet_match(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *tuple);

/*
 * SA API(s)
 */
void nss_ipsecmgr_copy_v4_sa(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v4 *sa);
void nss_ipsecmgr_copy_v6_sa(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v6 *sa);
void nss_ipsecmgr_copy_sa_data(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_data *data);
void nss_ipsecmgr_v4_sa2key(struct nss_ipsecmgr_sa_v4 *sa, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_v6_sa2key(struct nss_ipsecmgr_sa_v6 *sa, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_sa_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_sa_stats_update(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_entry *sa);

/*
 * SA alloc/lookup/flush API(s)
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_sa_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
struct nss_ipsecmgr_ref *nss_ipsecmgr_sa_lookup(struct nss_ipsecmgr_key *key);
void nss_ipsecmgr_sa_flush_all(struct nss_ipsecmgr_priv *priv);
/*
 * Stats op functions.
 */
ssize_t nss_ipsecmgr_netmask_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos);
ssize_t nss_ipsecmgr_sa_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos);
ssize_t nss_ipsecmgr_per_flow_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos);
ssize_t nss_ipsecmgr_per_flow_stats_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos);
ssize_t nss_ipsecmgr_flow_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos);

#endif /* __NSS_IPSECMGR_PRIV_H */
