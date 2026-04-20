/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
 * nss_nlethrx.h
 *      NSS Netlink eth_rx API definitions
 */
#ifndef __NSS_NLETHRX_H
#define __NSS_NLETHRX_H

bool nss_nlethrx_init(void);
bool nss_nlethrx_exit(void);

#if defined(CONFIG_NSS_NLETHRX)
#define NSS_NLETHRX_INIT nss_nlethrx_init
#define NSS_NLETHRX_EXIT nss_nlethrx_exit
#else
#define NSS_NLETHRX_INIT 0
#define NSS_NLETHRX_EXIT 0
#endif /* !CONFIG_NSS_NLETHRX */

#endif /* __NSS_NLETHRX_H */
