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

#include <linux/if_ether.h>

/*
 * ns_ppe_pppoe
 *	PPPoE offload information
 */
struct ppe_drv_pppoe {
	struct ppe_drv_l3_if *l3_if;		/* L3 interface corresponding to this pppoe entry */
	struct ppe_drv_tun_l3_if *tl_l3_if;	/* Tunnel L3 interface corresponding to this pppoe entry */
	struct kref ref;			/* Reference count */
	uint8_t port_bitmap;			/* TODO: Ports on which this session applies? */
	uint8_t index;				/* pppoe index number */
	uint8_t server_mac[ETH_ALEN]; 		/* PPPoE Server MAC */
	uint16_t session_id;			/* PPPoE session info */
	bool is_session_added;			/* PPPoE table add done */
};

void ppe_drv_pppoe_l3_if_deref(struct ppe_drv_pppoe *pppoe);
struct ppe_drv_l3_if *ppe_drv_pppoe_l3_if_get_and_ref(struct ppe_drv_pppoe *pppoe);

bool ppe_drv_pppoe_deref(struct ppe_drv_pppoe *pppoe);
struct ppe_drv_pppoe *ppe_drv_pppoe_ref(struct ppe_drv_pppoe *pppoe);
struct ppe_drv_tun_l3_if *ppe_drv_pppoe_tl_l3_if_get(struct ppe_drv_pppoe *pppoe);
bool ppe_drv_pppoe_tl_l3_if_attach(struct ppe_drv_pppoe *pppoe, struct ppe_drv_tun_l3_if *ptun_l3_if);
bool ppe_drv_pppoe_tl_l3_if_detach(struct ppe_drv_pppoe *pppoe);

void ppe_drv_pppoe_l3_if_attach(struct ppe_drv_pppoe *pppoe, struct ppe_drv_l3_if *l3_if);
void ppe_drv_pppoe_l3_if_detach(struct ppe_drv_pppoe *pppoe);
struct ppe_drv_l3_if *ppe_drv_pppoe_find_l3_if(uint16_t session_id, uint8_t *smac);

struct ppe_drv_pppoe *ppe_drv_pppoe_find_session_by_tl_l3_if(struct ppe_drv_tun_l3_if *tl_l3_if);
struct ppe_drv_pppoe *ppe_drv_pppoe_find_session(uint16_t session_id, uint8_t *smac);
struct ppe_drv_pppoe *ppe_drv_pppoe_alloc(uint16_t session_id, uint8_t *smac);

void ppe_drv_pppoe_entries_free(struct ppe_drv_pppoe *pppoe);
struct ppe_drv_pppoe *ppe_drv_pppoe_entries_alloc(void);
