/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#define PPE_DRV_TUN_L3_IF_MAX_ENTRIES FAL_TUNNEL_INTF_ENTRY_MAX

/*
 * ppe_drv_tun_l3_if
 *	TL L3 interface control table information
 */
struct ppe_drv_tun_l3_if {
	uint8_t index;		/* Entry index */
	struct kref ref;	/* Reference counter */
};

uint8_t ppe_drv_tun_l3_if_get_index(struct ppe_drv_tun_l3_if *tun_l3_if);
bool ppe_drv_tun_l3_if_configure(struct ppe_drv_tun_l3_if *tun_l3_if);

struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_alloc(struct ppe_drv *p);
struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_ref(struct ppe_drv_tun_l3_if *tun_l3_if);
bool ppe_drv_tun_l3_if_deref(struct ppe_drv_tun_l3_if *ptec);

void ppe_drv_tun_l3_if_entries_free(struct ppe_drv_tun_l3_if *ptun_l3_if);
struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_entries_alloc(struct ppe_drv *p);
