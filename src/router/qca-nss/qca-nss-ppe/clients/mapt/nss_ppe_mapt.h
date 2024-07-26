/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _NSS_PPE_MAPT_H_
#define _NSS_PPE_MAPT_H_

/*
 * NSS mapt debug macros
 */
#if (NSS_PPE_MAPT_DEBUG_LEVEL < 1)
#define nss_ppe_mapt_assert(fmt, args...)
#else
#define nss_ppe_mapt_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ppe_mapt_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_mapt_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_mapt_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_MAPT_DEBUG_LEVEL < 2)
#define nss_ppe_mapt_warning(s, ...)
#else
#define nss_ppe_mapt_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_MAPT_DEBUG_LEVEL < 3)
#define nss_ppe_mapt_info(s, ...)
#else
#define nss_ppe_mapt_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_MAPT_DEBUG_LEVEL < 4)
#define nss_ppe_mapt_trace(s, ...)
#else
#define nss_ppe_mapt_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#endif /* _NSS_PPE_MAPT_H_ */
