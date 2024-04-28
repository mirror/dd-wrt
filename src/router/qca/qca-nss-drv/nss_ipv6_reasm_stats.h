/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_IPV6_REASM_STATS_H
#define __NSS_IPV6_REASM_STATS_H

/*
 * IPv6 reasm node statistics
 */
enum nss_ipv6_reasm_stats {
	NSS_IPV6_REASM_STATS_ALLOC_FAILS = 0,
					/* Number of fragment queue allocation failures */
	NSS_IPV6_REASM_STATS_TIMEOUTS,
					/* Number of expired fragment queues */
	NSS_IPV6_REASM_STATS_DISCARDS,
					/* Number of fragment queues discarded due to malformed fragments*/
	NSS_IPV6_REASM_STATS_MAX,
};

/*
 * NSS IPv6 reasm statistics APIs
 */
extern void nss_ipv6_reasm_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_reasm_stats_sync *nirs);
extern void nss_ipv6_reasm_stats_dentry_create(void);

#endif /* __NSS_IPV6_REASM_STATS_H */
