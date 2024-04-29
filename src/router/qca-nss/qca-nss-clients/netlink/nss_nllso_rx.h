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
 * nss_nllso_rx.h
 *	NSS Netlink lso_rx API definitions
 */
#ifndef __NSS_NLLSO_RX_H
#define __NSS_NLLSO_RX_H

bool nss_nllso_rx_init(void);
bool nss_nllso_rx_exit(void);

#if defined(CONFIG_NSS_NLLSO_RX)
#define NSS_NLLSO_RX_INIT nss_nllso_rx_init
#define NSS_NLLSO_RX_EXIT nss_nllso_rx_exit
#else
#define NSS_NLLSO_RX_INIT 0
#define NSS_NLLSO_RX_EXIT 0
#endif /* !CONFIG_NSS_NLLSO_RX */

#endif /* __NSS_NLLSO_RX_H */
