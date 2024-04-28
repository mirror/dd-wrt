/*
 **************************************************************************
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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
 * nss_nlcryptov2.h
 *	NSS Netlink Crypto API definitions
 */
#ifndef __NSS_NLCRYPTOV2_H
#define __NSS_NLCRYPTOV2_H
#define NSS_NLCRYPTOV2_HDR_POOL_SZ 1
#define NSS_NLCRYPTOV2_DEFAULT_HDR_SZ 512
#define NSS_NLCRYPTOV2_TIMEOUT 10

#if defined(CONFIG_NSS_NLCRYPTOV2)
#define NSS_NLCRYPTOV2_INIT nss_nlcryptov2_init
#define NSS_NLCRYPTOV2_EXIT nss_nlcryptov2_exit
#else
#define NSS_NLCRYPTOV2_INIT 0
#define NSS_NLCRYPTOV2_EXIT 0
#endif /* !CONFIG_NSS_NLCRYPTOV2 */

bool nss_nlcryptov2_init(void);
bool nss_nlcryptov2_exit(void);

#endif /* __NSS_NLCRYPTOV2_H */
