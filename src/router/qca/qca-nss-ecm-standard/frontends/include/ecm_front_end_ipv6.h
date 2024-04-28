/*
 **************************************************************************
 * Copyright (c) 2014-2016, The Linux Foundation.  All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

extern int ecm_front_end_ipv6_stopped;	/* When non-zero further traffic will not be processed */

#ifdef ECM_MULTICAST_ENABLE
extern int ecm_front_end_ipv6_mc_stopped;	/* When non-zero further traffic will not be processed */
#endif

#ifdef ECM_FRONT_END_NSS_ENABLE
#include "ecm_nss_ipv6.h"
#else
static inline int ecm_nss_ipv6_init(struct dentry *dentry)
{
	/*
	 * Just return if nss front end is not enabled
	 */
	return 0;
}

static inline void ecm_nss_ipv6_exit(void)
{
	/*
	 * Just return if nss front end is not enabled
	 */
	return;
}
#endif

#ifdef ECM_FRONT_END_SFE_ENABLE
#include "ecm_sfe_ipv6.h"
#else
static inline int ecm_sfe_ipv6_init(struct dentry *dentry)
{
	/*
	 * Just return if sfe front end is not enabled
	 */
	return 0;
}

static inline void ecm_sfe_ipv6_exit(void)
{
	/*
	 * Just return if sfe front end is not enabled
	 */
	return;
}
#endif

/*
 * IPv6 rule sync reasons.
 */
enum ecm_front_end_ipv6_rule_sync_reason {
	ECM_FRONT_END_IPV6_RULE_SYNC_REASON_STATS = 0,	/* Sync is to synchronize stats */
	ECM_FRONT_END_IPV6_RULE_SYNC_REASON_FLUSH,	/* Sync is to flush a cache entry */
	ECM_FRONT_END_IPV6_RULE_SYNC_REASON_EVICT,	/*Sync is to evict a cache entry */
	ECM_FRONT_END_IPV6_RULE_SYNC_REASON_DESTROY	/* Sync is to destroy a cache entry */
};

extern void ecm_front_end_ipv6_stop(int num);
extern int ecm_front_end_ipv6_init(struct dentry *dentry);
extern void ecm_front_end_ipv6_exit(void);

