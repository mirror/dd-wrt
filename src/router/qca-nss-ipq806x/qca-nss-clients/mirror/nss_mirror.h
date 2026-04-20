/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/etherdevice.h>

#include <nss_api_if.h>
#include <nss_cmn.h>

#if (NSS_MIRROR_DEBUG_LEVEL < 1)
#define nss_mirror_assert(fmt, args...)
#else
#define nss_mirror_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif /* NSS_MIRROR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_mirror_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_mirror_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_mirror_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_MIRROR_DEBUG_LEVEL < 2)
#define nss_mirror_warn(s, ...)
#else
#define nss_mirror_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_MIRROR_DEBUG_LEVEL < 3)
#define nss_mirror_info(s, ...)
#else
#define nss_mirror_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_MIRROR_DEBUG_LEVEL < 4)
#define nss_mirror_trace(s, ...)
#else
#define nss_mirror_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * Mirror configure modes.
 */
#define NSS_MIRROR_MODE_NONE 0x0
#define NSS_MIRROR_MODE_INGRESS_PMC 0x1

/*
 * nss_mirror_instance_priv
 *	Private structure of mirror net device.
 */
struct nss_mirror_instance_priv {
	struct rtnl_link_stats64 stats;			/* Netdev stats. */
	uint32_t mirror_instance_if_num;		/* NSS interface number for this mirror device. */
	enum nss_mirror_pkt_clone_point mirror_point;	/* Point in the packet to copy from. */
	uint16_t mirror_size;				/* Number of bytes to copy. */
	uint16_t mirror_offset;				/* Copy offset. */
	uint8_t rule_config_mode;			/* Configure mode. */
	uint8_t mirror_arr_index;			/* Netdev index in mirror array. */
};

/*
 * nss_mirror_display_info()
 *	API to display configure information for the given mirror device.
 */
extern void nss_mirror_display_info(struct net_device *mirror_dev);

/*
 * nss_mirror_display_all_info()
 *	API to display configure information for all mirror devices.
 */
extern void nss_mirror_display_all_info(void);

/*
 * nss_mirror_reset_if_nexthop()
 *	API to send reset nexthop command to NSS firmware.
 * TODO: Remove this API and its usage and use NSS driver API, once
 * similar API is added into NSS driver.
 */
extern nss_tx_status_t nss_mirror_reset_if_nexthop(uint32_t if_num);

/*
 * nss_mirror_deconfigure_mirror()
 *	API to deconfigure mirror device.
 */
extern int nss_mirror_deconfigure_mirror(struct net_device *mirror_dev);

/*
 * nss_mirror_destroy_all()
 *	API to destroy all the configured mirror devices.
 */
extern int nss_mirror_destroy_all(void);
