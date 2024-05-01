/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_IPSEC_XFRM_H
#define __NSS_IPSEC_XFRM_H

#define NSS_IPSEC_XFRM_TUN_DB_MAX 128
#define NSS_IPSEC_XFRM_FLOW_DB_MAX 1024

#define NSS_IPSEC_XFRM_TUNNEL_FREE_TIMEOUT_MSECS 10000
#define NSS_IPSEC_XFRM_TUNNEL_FREE_TIMEOUT msecs_to_jiffies(NSS_IPSEC_XFRM_TUNNEL_FREE_TIMEOUT_MSECS)

#define NSS_IPSEC_XFRM_DEBUG_LVL_ERROR 1
#define NSS_IPSEC_XFRM_DEBUG_LVL_WARN 2
#define NSS_IPSEC_XFRM_DEBUG_LVL_INFO 3
#define NSS_IPSEC_XFRM_DEBUG_LVL_TRACE 4

#define nss_ipsec_xfrm_info_always(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ipsec_xfrm_err(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsec_xfrm_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsec_xfrm_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ipsec_xfrm_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else

/*
 * Statically compile messages at different levels
 */
#define nss_ipsec_xfrm_err(s, ...) {	\
	if (NSS_IPSEC_XFRM_DEBUG_LEVEL > NSS_IPSEC_XFRM_DEBUG_LVL_ERROR) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_xfrm_warn(s, ...) {	\
	if (NSS_IPSEC_XFRM_DEBUG_LEVEL > NSS_IPSEC_XFRM_DEBUG_LVL_WARN) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_xfrm_info(s, ...) {	\
	if (NSS_IPSEC_XFRM_DEBUG_LEVEL > NSS_IPSEC_XFRM_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_xfrm_trace(s, ...) {	\
	if (NSS_IPSEC_XFRM_DEBUG_LEVEL > NSS_IPSEC_XFRM_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */

#define NSS_IPSEC_XFRM_PRINT_PAGES 2
#define NSS_IPSEC_XFRM_PRINT_BYTES(bytes) ((((bytes) * BITS_PER_BYTE) / 4) + 4)
#define NSS_IPSEC_XFRM_PRINT_DWORD NSS_IPSEC_XFRM_PRINT_BYTES(8)
#define NSS_IPSEC_XFRM_PRINT_WORD NSS_IPSEC_XFRM_PRINT_BYTES(4)
#define NSS_IPSEC_XFRM_PRINT_SHORT NSS_IPSEC_XFRM_PRINT_BYTES(2)
#define NSS_IPSEC_XFRM_PRINT_BYTE NSS_IPSEC_XFRM_PRINT_BYTES(1)
#define NSS_IPSEC_XFRM_PRINT_IPADDR (NSS_IPSEC_XFRM_PRINT_WORD * 4)
#define NSS_IPSEC_XFRM_PRINT_EXTRA 64      /* Bytes */

/*
 * Statistics dump information
 */
struct nss_ipsec_xfrm_print {
	char *str;              /* Name of variable. */
	ssize_t var_size;       /* Size of variable in bytes. */
};

struct nss_ipsec_xfrm_fallback {
	struct xfrm_state_afinfo *v4;
	struct xfrm_state_afinfo *v6;
};

/*
 * nss_ipsec_xfrm_drv_stats
 *	NSS IPSec DRV stats.
 */
struct nss_ipsec_xfrm_drv_stats {
	atomic64_t encap_nss;		/* Number of Packets sent(queued) to NSS for encap */
	atomic64_t encap_drop;		/* Number of Packets dropped that were trapped for encap */
	atomic64_t decap_nss;		/* Number of Packets queued to NSS for decap */
	atomic64_t decap_drop;		/* Number of Packets dropped that were trapped for decap */
	atomic64_t inner_except;	/* Number of inner exceptions that were indicated to stack*/
	atomic64_t inner_drop; 		/* Number of inner exceptions that were dropped*/
	atomic64_t outer_except;        /* Number of outer exceptions that were sent out*/
	atomic64_t outer_drop;          /* Number of outer exceptions that were dropped */
	atomic64_t tun_alloced;		/* Number of tun objects created */
	atomic64_t tun_dealloced;	/* Number of tun objects created */
	atomic64_t tun_freed;		/* Number of tun objects released(freed) */
	atomic64_t sa_alloced;		/* Number of sa objects created */
	atomic64_t sa_dealloced;	/* Number of sa objects created */
	atomic64_t sa_freed;		/* Number of sa objects released(freed) */
	atomic64_t flow_alloced;	/* Number of flow objects created */
	atomic64_t flow_dealloced;	/* Number of flow objects created */
	atomic64_t flow_freed;		/* Number of flow objects deleted */
};

/*
 * IPSec xfrm plugin instance
 */
struct nss_ipsec_xfrm_drv {
	struct list_head tun_db[NSS_IPSEC_XFRM_TUN_DB_MAX];	/* Tunnel Hash Database */
	struct list_head flow_db[NSS_IPSEC_XFRM_FLOW_DB_MAX];	/* Flow Hash Database */
	struct nss_ipsec_xfrm_drv_stats stats;			/* Statistics */
	struct nss_ipsec_xfrm_fallback xsa;			/* Pointers to original xfrm_state_afinfo instances */
	struct completion complete;				/* Completion for num of active tunnels to go zero. */
	struct dentry *dentry;					/* Debugfs root directory. */
	size_t stats_len;					/* Size of stats */
	rwlock_t lock;						/* Lock for DB operations */
	atomic_t num_tunnels;					/* Number of active tunnels */
	uint32_t hash_nonce;					/* One time random number */
};

/*
 * nss_ipsec_xfrm_get_hash()
 *	Computes hash code for a given key
 */
static inline uint32_t nss_ipsec_xfrm_get_hash(uint32_t *key, uint32_t key_len, uint32_t seed)
{
	return jhash2(key, key_len / sizeof(uint32_t), seed);

}

void nss_ipsec_xfrm_update_stats(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsecmgr_sa_stats *stats);
#endif /* !__NSS_IPSEC_XFRM_H */
