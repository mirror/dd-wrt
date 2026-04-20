/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file nss_ipsec_klips.h
 * 	 cfi plugin header file
 */
#ifndef __NSS_IPSEC_KLIPS_H
#define __NSS_IPSEC_KLIPS_H

/**
 * @file klips plugin for ipsec manager.
 */

#define NSS_IPSEC_KLIPS_NON_ESP_MARKER 0x0000

typedef int (*sk_encap_rcv_method_t)(struct sock *sk, struct sk_buff *skb);

#define NSS_IPSEC_KLIPS_DEBUG_LVL_ERROR 1
#define NSS_IPSEC_KLIPS_DEBUG_LVL_WARN 2
#define NSS_IPSEC_KLIPS_DEBUG_LVL_INFO 3
#define NSS_IPSEC_KLIPS_DEBUG_LVL_TRACE 4

#define nss_ipsec_klips_info_always(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ipsec_klips_err(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsec_klips_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsec_klips_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ipsec_klips_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else

/*
 * Statically compile messages at different levels
 */
#define nss_ipsec_klips_err(s, ...) {	\
	if (NSS_IPSEC_KLIPS_DEBUG_LEVEL > NSS_IPSEC_KLIPS_DEBUG_LVL_ERROR) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_klips_warn(s, ...) {	\
	if (NSS_IPSEC_KLIPS_DEBUG_LEVEL > NSS_IPSEC_KLIPS_DEBUG_LVL_WARN) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_klips_info(s, ...) {	\
	if (NSS_IPSEC_KLIPS_DEBUG_LEVEL > NSS_IPSEC_KLIPS_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsec_klips_trace(s, ...) {	\
	if (NSS_IPSEC_KLIPS_DEBUG_LEVEL > NSS_IPSEC_KLIPS_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */

#define NSS_IPSEC_KLIPS_BITS2BYTE(x) ((x) / BITS_PER_BYTE) /**< Bits to Bytes */
#define nss_ipsec_klips_assert(expr) BUG_ON(!expr)

#if !defined (CONFIG_NSS_IPSEC_KLIPS_DBG)
#define nss_ipsec_klips_dbg(fmt, arg...)
#define nss_ipsec_klips_dbg_skb(skb, limit)
#define nss_ipsec_klips_dbg_data(data, len, c)

#else
#define nss_ipsec_klips_dbg(fmt, arg...)     printk(KERN_DEBUG fmt, ## arg)

/**
 * @brief print the skb contents till the limit
 *
 * @param skb[IN] skb data to print from
 * @param limit[IN] limit for printing
 */
static inline void nss_ipsec_klips_dbg_skb(struct sk_buff *skb, uint32_t limit)
{
	uint32_t len;
	int i;

	len = (skb->len < limit) ? skb->len : limit;

	printk("-- skb dump --\n");
	for (i = 0; i < len; i++) {
		printk("0x%02x ",skb->data[i]);
	}
	printk("\n");
}

/**
 * @brief print data till length
 *
 * @param data[IN] data to print
 * @param len[IN] length of data to print
 * @param c[IN] charater to use between prints
 */
static inline void nss_ipsec_klips_dbg_data(uint8_t *data, uint16_t len, uint8_t c)
{
	int i = 0;

	printk("-- data dump --\n");
	for (i = 0; i < len; i++) {
		printk("0x%02x%c", data[i], c);
	}
	printk("\n");
}
#endif

#endif /* !__NSS_IPSEC_KLIPS_H */
