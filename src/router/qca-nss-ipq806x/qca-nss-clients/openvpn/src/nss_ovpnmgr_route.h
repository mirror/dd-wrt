/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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
 * nss_ovpnmgr_route.h
 */
#ifndef __NSS_OVPNMGR_ROUTE__H
#define __NSS_OVPNMGR_ROUTE__H

/*
 * nss_ovpnmgr_route
 */
struct nss_ovpnmgr_route {
	struct list_head list;			/* List of Routes configured in this tunnel. */
	struct nss_ovpnmgr_route_tuple rt;	/* Route tuple. */
	uint32_t in_use;			/* Increments when ecm stats update updated from NSS FW */
};

struct nss_ovpnmgr_route *nss_ovpnmgr_route_find(struct list_head *rt_list, struct nss_ovpnmgr_route_tuple *rt);
int nss_ovpnmgr_route_set_active(struct list_head *rt_list, struct nss_ovpnmgr_route_tuple *rt);
#endif /* __NSS_OVPNMGR_ROUTE__H */
