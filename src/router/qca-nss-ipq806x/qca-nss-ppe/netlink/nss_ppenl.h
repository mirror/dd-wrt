/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_ppenl_.h
 *	NSS PPE Netlink internal definitions
 */
#ifndef __NSS_PPENL_H
#define __NSS_PPENL_H

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define nss_ppenl_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppenl_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppenl_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (NSS_PPENL_DEBUG_LEVEL < 2)
#define nss_ppenl_warn(s, ...)
#else
#define nss_ppenl_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPENL_DEBUG_LEVEL < 3)
#define nss_ppenl_info(s, ...)
#else
#define nss_ppenl_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPENL_DEBUG_LEVEL < 4)
#define nss_ppenl_trace(s, ...)
#else
#define nss_ppenl_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * debug message for module init and exit
 */
#define nss_ppenl_info_always(s, ...) printk(KERN_INFO"%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)


#define NSS_NETLINK_INIT_FAMILY(family, pre_fn, post_fn) {	\
	.id = GENL_ID_GENERATE,	\
	.name = NSS_NETLINK_##family##FAMILY,	\
	.hdrsize = 0,	\
	.version = NSS_NETLINK_VER,	\
	.maxattr = NSS_NETLINK_##family##_CMD_MAX,	\
	.netnsok = true,	\
	.pre_doit = pre_fn,	\
	.post_doit = post_fn,	\
}

#define NSS_NETLINK_INIT_POLICY(cmd, ds) {	\
	.type = NLA_BINARY,	\
	.len = sizeof(struct ##ds),	\
}


typedef bool (*nss_ppenl_fn_t)(void);

/*
 * *************
 * Generic API(s)
 * *************
 */

/*
 * Copy NSS NL message buffer into a new SKB
 */
struct sk_buff *nss_ppenl_copy_msg(struct sk_buff *orig);

/*
 * Get NSS NL message pointer for a given command
 */
struct nss_ppenl_cmn *nss_ppenl_get_msg(struct genl_family *family, struct genl_info *info, uint16_t cmd);

/*
 * returns the start of nss_ppenl_ payload
 */
void *nss_ppenl_get_data(struct sk_buff *skb);

/*
 * unicast response to the user
 */
int nss_ppenl_ucast_resp(struct sk_buff *skb);

#endif /* __NSS_PPENL_H */
