/* Copyright (c) 2013, 2016, The Linux Foundation. All rights reserved.
 *
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
#ifndef __NSS_CRYPTO_HLOS_H
#define __NSS_CRYPTO_HLOS_H

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38) && !defined(AUTOCONF_INCLUDED)
#include<linux/config.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/random.h>
#include <linux/skbuff.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/slab.h>
#include <linux/llist.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>


#define NSS_CRYPTO_DEBUG_LVL_ERROR 1
#define NSS_CRYPTO_DEBUG_LVL_WARN 2
#define NSS_CRYPTO_DEBUG_LVL_INFO 3
#define NSS_CRYPTO_DEBUG_LVL_TRACE 4

#define nss_crypto_info_always(s, ...) pr_notice("<NSS-CRYPTO>:" s, ##__VA_ARGS__)

#define nss_crypto_err(s, ...) do { \
	if (net_ratelimit()) {  \
		pr_alert("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_crypto_warn(s, ...) do { \
	if (net_ratelimit()) {  \
		pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_crypto_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_crypto_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#define nss_crypto_info(s, ...) {	\
	if (NSS_CRYPTO_DEBUG_LEVEL > NSS_CRYPTO_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_crypto_trace(s, ...) {	\
	if (NSS_CRYPTO_DEBUG_LEVEL > NSS_CRYPTO_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */

#endif /* __NSS_CRYPTO_HLOS_H */
