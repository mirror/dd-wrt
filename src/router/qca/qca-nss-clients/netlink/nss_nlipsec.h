/*
 **************************************************************************
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
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
 * nss_nlipsec.h
 *	NSS Netlink IPsec API definitions
 */
#ifndef __NSS_NLIPSEC_H
#define __NSS_NLIPSEC_H


bool nss_nlipsec_init(void);
bool nss_nlipsec_exit(void);

#if defined(CONFIG_NSS_NLIPSEC)
#define NSS_NLIPSEC_INIT nss_nlipsec_init
#define NSS_NLIPSEC_EXIT nss_nlipsec_exit
#else
#define NSS_NLIPSEC_INIT 0
#define NSS_NLIPSEC_EXIT 0
#endif /* !CONFIG_NSS_NLIPSEC */

#endif /* __NSS_NLIPSEC_H */
