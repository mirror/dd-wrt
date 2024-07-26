/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __NSS_PPE_VXLANMGR_TUN_STATS_H
#define __NSS_PPE_VXLANMGR_TUN_STATS_H

/*
 * VxLAN statistics APIs
 */
extern void nss_ppe_vxlanmgr_tun_stats_dentry_deinit(void);
extern bool nss_ppe_vxlanmgr_tun_dentry_init(void);
extern void nss_ppe_vxlanmgr_tun_stats_dentry_remove(struct nss_ppe_vxlanmgr_tun_ctx *tun_ctx);
extern bool nss_ppe_vxlanmgr_tun_stats_dentry_create(struct nss_ppe_vxlanmgr_tun_ctx *tun_ctx);

#endif /* __NSS_PPE_VXLANMGR_TUN_STATS_H */
