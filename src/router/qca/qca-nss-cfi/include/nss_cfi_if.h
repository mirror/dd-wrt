/* Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
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
 *
 *
 */

/**
 * @file nss_cfi_if.h
 * 	 nss cfi interface file
 */
#ifndef __NSS_CFI_IF_H
#define __NSS_CFI_IF_H

/**
 * @file Interface to communicate OCF specific data to Core specific data.
 */
#define NSS_CFI_DEBUG_LVL_ERROR 1
#define NSS_CFI_DEBUG_LVL_WARN 2
#define NSS_CFI_DEBUG_LVL_INFO 3
#define NSS_CFI_DEBUG_LVL_TRACE 4

#define nss_cfi_info_always(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_cfi_err(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_cfi_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_cfi_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_cfi_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#define nss_cfi_err(s, ...) {	\
	if (NSS_CFI_DEBUG_LEVEL > NSS_CFI_DEBUG_LVL_ERROR) {	\
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_cfi_warn(s, ...) {	\
	if (NSS_CFI_DEBUG_LEVEL > NSS_CFI_DEBUG_LVL_WARN) {	\
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_cfi_info(s, ...) {	\
	if (NSS_CFI_DEBUG_LEVEL > NSS_CFI_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_cfi_trace(s, ...) {	\
	if (NSS_CFI_DEBUG_LEVEL > NSS_CFI_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */

#define nss_cfi_assert(expr) BUG_ON(!expr)

#if !defined (CONFIG_NSS_CFI_DBG)
#define nss_cfi_dbg(fmt, arg...)
#define nss_cfi_dbg_skb(skb, limit)
#define nss_cfi_dbg_data(data, len, c)

#else
#define nss_cfi_dbg(fmt, arg...)     printk(KERN_DEBUG fmt, ## arg)



/**
 * @brief print the skb contents till the limit
 *
 * @param skb[IN] skb data to print from
 * @param limit[IN] limit for printing
 */
static inline void nss_cfi_dbg_skb(struct sk_buff *skb, uint32_t limit)
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
static inline void nss_cfi_dbg_data(uint8_t *data, uint16_t len, uint8_t c)
{
	int i = 0;

	printk("-- data dump --\n");
	for (i = 0; i < len; i++) {
		printk("0x%02x%c", data[i], c);
	}
	printk("\n");
}
#endif

/**
 * @brief Structure representing crypto control data needed by IPsec
 *        fast path rule
 */
struct nss_cfi_crypto_info {
	uint16_t sid;			/**< Crypto Session index */

#if defined(NSS_CFI_IPQ807X_SUPPORT) || defined(NSS_CFI_IPQ60XX_SUPPORT)
	enum nss_crypto_cmn_algo algo;	/**< supported algorithm */
	uint8_t pad[2];			/**< padding */
#else
	uint8_t cipher_algo;		/**< Cipher algorithm */
	uint8_t auth_algo;		/**< Authentication algorithm */
#endif
	uint8_t hash_len;		/**< Authentication data length */
	uint8_t res[3];			/**< Reserve for 4 byte alignment */
};

/**
 * @brief Handler to trap IPsec data packets
 */
typedef int32_t (*nss_cfi_data_trap_t)(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto);

/**
 * @brief Handler to trap IPsec session information
 */
typedef int32_t (*nss_cfi_session_trap_t)(uint32_t session_idx);

/**
 * @brief Register the IPsec trap handlers for trapping packets before encryption and
 * 	  after decryption. As we want to look at all the headers in plain text form
 *
 * @param encrypt_fn[IN] trap function for encryption direction mostly encap rules
 * @param decrypt_fn[IN] trap function for decryption direction mostly decap rules
 * @param session_fn[IN] trap function for session related information.
 */
void nss_cfi_ocf_register_ipsec(nss_cfi_data_trap_t encrypt_fn, nss_cfi_data_trap_t decrypt_fn, nss_cfi_session_trap_t session_fn);

/**
 * @brief Unregister the Ipsec trap handlers.
 */
void nss_cfi_ocf_unregister_ipsec(void);

#endif /* !__NSS_CFI_IF_H */
