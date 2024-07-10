/*
 **************************************************************************
 * Copyright (c) 2015,2018, 2020, The Linux Foundation. All rights reserved.
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
 * nss_nl.h
 *	NSS Netlink internal definitions
 */
#ifndef __NSS_NL_H
#define __NSS_NL_H

#define NSS_NL_DEBUG_LVL_ERROR 1
#define NSS_NL_DEBUG_LVL_WARN 2
#define NSS_NL_DEBUG_LVL_INFO 3
#define NSS_NL_DEBUG_LVL_TRACE 4


#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_nl_error(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_nl_warn(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_nl_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_nl_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_nl_hex_dump_bytes(prefix_str, prefix_type, buf, len)	\
	dynamic_hex_dump(prefix_str, prefix_type, 16, 1, buf, len, true)
#else
/*
 * Statically compile messages at different levels
 */
#define nss_nl_error(s, ...) {	\
	if (NSS_NL_DEBUG_LEVEL > NSS_NL_DEBUG_LVL_ERROR) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_nl_warn(s, ...) {	\
	if (NSS_NL_DEBUG_LEVEL > NSS_NL_DEBUG_LVL_WARN) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_nl_info(s, ...) {	\
	if (NSS_NL_DEBUG_LEVEL > NSS_NL_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_nl_hex_dump_bytes(prefix_str, prefix_type, buf, len) {	\
	if (NSS_NL_DEBUG_LEVEL > NSS_NL_DEBUG_LVL_INFO) {	\
		print_hex_dump_bytes(prefix_str, prefix_type, buf, len);	\
	}	\
}
#define nss_nl_trace(s, ...) {	\
	if (NSS_NL_DEBUG_LEVEL > NSS_NL_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#endif /* !CONFIG_DYNAMIC_DEBUG */

/*
 * debug message for module init and exit
 */
#define nss_nl_info_always nss_nl_info

//#define nss_nl_info_always(s, ...) printk(KERN_INFO"%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)


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


typedef bool (*nss_nl_fn_t)(void);

/*
 * *************
 * Generic API(s)
 * *************
 */

/*
 * Allocate NSS NL message buffer, this returns the message SKB and start of the payload pointer
 */
struct sk_buff *nss_nl_new_msg(struct genl_family *family, uint8_t cmd);

/*
 * Copy NSS NL message buffer into a new SKB
 */
struct sk_buff *nss_nl_copy_msg(struct sk_buff *orig);

/*
 * Get NSS NL message pointer for a given command
 */
struct nss_nlcmn *nss_nl_get_msg(struct genl_family *family, struct genl_info *info, uint16_t cmd);

/*
 * returns the start of NSS_NL payload
 */
void *nss_nl_get_data(struct sk_buff *skb);

/*
 * unicast response to the user
 */
int nss_nl_ucast_resp(struct sk_buff *skb);

/*
 * multicast response to the user
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
int nss_nl_mcast_event(struct genl_family *family, struct sk_buff *skb);
#else
int nss_nl_mcast_event(struct genl_multicast_group *grp, struct sk_buff *skb);
#endif
#endif /* __NSS_NL_H */
