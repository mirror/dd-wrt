/*
 **************************************************************************
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#include <linux/version.h>
#include <net/act_api.h>

#define NSS_MIRRED_TAB_MASK 7

/*
 * nss_mirred_tcf
 *	nss mirred internal structure.
 */
struct nss_mirred_tcf {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	struct tcf_common common;		/* Common filter structure */
#else
	struct tc_action common;		/* Common filter structure */
#endif
	__u32 tcfm_to_ifindex;			/* Index number of device to which
						 * traffic will be redirected.
						 */
	__u32 tcfm_from_ifindex;		/* Index number of device from which
						 * traffic will be redirected.
						 */
	struct net_device __rcu *tcfm_dev;	/* net device pointer of the device
						 * to which traffic will be redirected.
						 */
	struct list_head tcfm_list;		/* list for the nss mirred action */
};

/*
 * To get the pointer of nss mirred action structure from the common
 * tc_action structure pointer.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
#define nss_mirred_get(a) \
	container_of(a->priv, struct nss_mirred_tcf, common)
#else
#define nss_mirred_get(a) ((struct nss_mirred_tcf *)a)
#endif

