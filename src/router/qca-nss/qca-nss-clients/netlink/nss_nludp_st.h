/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * nss_nludp_st.h
 *      NSS Netlink udp_st API definitions
 */
#ifndef __NSS_NLUDP_ST_H
#define __NSS_NLUDP_ST_H

#if defined(CONFIG_NSS_NLUDP_ST) && CONFIG_NSS_NLUDP_ST > 0
bool nss_nludp_st_init(void);
bool nss_nludp_st_exit(void);
#define NSS_NLUDP_ST_INIT nss_nludp_st_init
#define NSS_NLUDP_ST_EXIT nss_nludp_st_exit
#else
#define NSS_NLUDP_ST_INIT 0
#define NSS_NLUDP_ST_EXIT 0
#endif /* !CONFIG_NSS_NLUDP_ST */

#endif /* __NSS_NLUDP_ST_H */
