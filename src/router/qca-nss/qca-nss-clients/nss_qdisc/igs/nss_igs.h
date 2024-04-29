/*
 **************************************************************************
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#define NSS_IGS_DEBUG_LEVEL_ERROR 1
#define NSS_IGS_DEBUG_LEVEL_WARN 2
#define NSS_IGS_DEBUG_LEVEL_INFO 3
#define NSS_IGS_DEBUG_LEVEL_TRACE 4

/*
 * Debug message for module init and exit
 */
#define nss_igs_info_always(s, ...) printk(KERN_INFO"%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

/*
 * Error and warn message will be enabled by default in Makefile
 */
#if (NSS_IGS_DEBUG_LEVEL < NSS_IGS_DEBUG_LEVEL_ERROR)
#define nss_igs_assert(s, ...)
#define nss_igs_error(s, ...)
#else
#define nss_igs_assert(c, s, ...) { if (!(c)) { pr_emerg("ASSERT: %s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG(); } }
#define nss_igs_error(s, ...) pr_err("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_IGS_DEBUG_LEVEL < NSS_IGS_DEBUG_LEVEL_WARN)
#define nss_igs_warning(s, ...)
#else
#define nss_igs_warning(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_igs_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_igs_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#if (NSS_IGS_DEBUG_LEVEL < NSS_IGS_DEBUG_LEVEL_INFO)
#define nss_igs_info(s, ...)
#else
#define nss_igs_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_IGS_DEBUG_LEVEL < NSS_IGS_DEBUG_LEVEL_TRACE)
#define nss_igs_trace(s, ...)
#else
#define nss_igs_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif
