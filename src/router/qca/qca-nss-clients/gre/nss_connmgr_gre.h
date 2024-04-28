/*
 **************************************************************************
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

/*
 * nss_connnmgr_gre.h
 *
 * Header file implementaion of nss_connmgr_gre
 */

#ifndef _NSS_CONNMGR_GRE_H_
#define _NSS_CONNMGR_GRE_H_

/*
 * GRE debug macros
 */
#if (NSS_GRE_DEBUG_LEVEL < 1)
#define nss_connmgr_gre_assert(fmt, args...)
#else
#define nss_connmgr_gre_assert(c)  BUG_ON(!(c));
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_connmgr_gre_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_gre_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_gre_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_GRE_DEBUG_LEVEL < 2)
#define nss_connmgr_gre_warning(s, ...)
#else
#define nss_connmgr_gre_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_GRE_DEBUG_LEVEL < 3)
#define nss_connmgr_gre_info(s, ...)
#else
#define nss_connmgr_gre_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_GRE_DEBUG_LEVEL < 4)
#define nss_connmgr_gre_trace(s, ...)
#else
#define nss_connmgr_gre_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif
#define nss_connmgr_gre_error(s, ...) pr_err("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#define GRE_HDR_PAD_LEN 2

typedef struct nss_gre_info nss_connmgr_gre_priv_t;

uint32_t nss_connmgr_gre_get_hlen(struct nss_connmgr_gre_cfg *cfg);
void nss_connmgr_gre_set_gre_flags(struct nss_connmgr_gre_cfg *cfg,
					   uint16_t *o_flags, uint16_t *i_flags);

uint32_t nss_connmgr_gre_get_nss_config_flags(uint16_t o_flags, uint16_t i_flags,
						  uint8_t tos, uint8_t ttl,
						  uint16_t frag_off);
int nss_connmgr_gre_set_wifi_next_hop(struct net_device *wifi_vdev);

int nss_connmgr_gre_v4_set_config(struct net_device *dev, struct nss_connmgr_gre_cfg *cfg);
int nss_connmgr_gre_v6_set_config(struct net_device *dev, struct nss_connmgr_gre_cfg *cfg);

int nss_connmgr_gre_v4_get_config(struct net_device *dev, struct nss_gre_msg *req, struct net_device **next_dev, bool hold);
int nss_connmgr_gre_v6_get_config(struct net_device *dev, struct nss_gre_msg *req, struct net_device **next_dev, bool hold);

void nss_connmgr_gre_tap_v4_outer_exception(struct net_device *dev, struct sk_buff *skb);
void nss_connmgr_gre_tap_v6_outer_exception(struct net_device *dev, struct sk_buff *skb);

void nss_connmgr_gre_tun_v4_outer_exception(struct net_device *dev, struct sk_buff *skb);
void nss_connmgr_gre_tun_v6_outer_exception(struct net_device *dev, struct sk_buff *skb);

#endif
