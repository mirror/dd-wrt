/*
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

/*
 * nss_ppe_bridge_mgr.h
 */

#ifndef __NSS_PPE_BRIDGE_MGR_H
#define __NSS_PPE_BRIDGE_MGR_H
/*
 * Bridge-manager functionality APIs exported
 */
int nss_ppe_bridge_mgr_leave_bridge(struct net_device *dev, struct net_device *bridge_dev);
int nss_ppe_bridge_mgr_join_bridge(struct net_device *dev, struct net_device *bridge_dev);
#endif /* __NSS_PPE_BRIDGE_MGR_H */
