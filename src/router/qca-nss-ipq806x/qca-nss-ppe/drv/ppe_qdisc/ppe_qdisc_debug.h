/*
 * Copyright (c) 2014-2018, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_QDISC_DEBUG_H_
#define _PPE_QDISC_DEBUG_H_

/*
 * PPE Qdisc debug macros
 */
#if (PPE_QDISC_DEBUG_LEVEL < 1)
#define ppe_qdisc_assert(fmt, args...)
#else
#define ppe_qdisc_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define ppe_qdisc_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ppe_qdisc_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ppe_qdisc_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */

/*
 * Statically compile messages at different levels
 */
#if (PPE_QDISC_DEBUG_LEVEL < 2)
#define ppe_qdisc_warning(s, ...)
#else
#define ppe_qdisc_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_QDISC_DEBUG_LEVEL < 3)
#define ppe_qdisc_info(s, ...)
#else
#define ppe_qdisc_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_QDISC_DEBUG_LEVEL < 4)
#define ppe_qdisc_trace(s, ...)
#else
#define ppe_qdisc_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#endif
