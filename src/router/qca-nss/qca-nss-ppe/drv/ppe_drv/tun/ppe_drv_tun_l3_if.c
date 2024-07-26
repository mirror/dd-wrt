/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <fal_tunnel.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_l3_if_dump
 *	Dump TL_L3_IF table instance
 */
static void ppe_drv_tun_l3_if_dump(struct ppe_drv_tun_l3_if *tun_l3_if)
{

	fal_tunnel_intf_t tun_l3_if_cfg = {0};
	sw_error_t err;

	err = fal_tunnel_intf_get(PPE_DRV_SWITCH_ID, tun_l3_if->index, &tun_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Failed to get TL_L3_IF config at index %u", tun_l3_if, tun_l3_if->index);
		return;
	}

	ppe_drv_trace("%p: TL_L3_IF_TBL_INDEX: %d, IPV4_DECAP_EN[%d], IPV6_DECAP_EN[%d]",
		tun_l3_if, tun_l3_if->index,
			tun_l3_if_cfg.ipv4_decap_en,
			tun_l3_if_cfg.ipv6_decap_en);

	ppe_drv_trace("%p: DMAC_CHECK_DIS[%d], TTL_EXCEED_CMD[%d], TTL_EXCEED_DE_ACCE[%d], LPM_EN[%d], MIN_IPV6_MTU[%d]",
			tun_l3_if,
			tun_l3_if_cfg.dmac_check_en,
			tun_l3_if_cfg.ttl_exceed_action,
			tun_l3_if_cfg.ttl_exceed_deacce_en,
			tun_l3_if_cfg.lpm_en,
			tun_l3_if_cfg.mini_ipv6_mtu);
}

/*
 * ppe_drv_tun_l3_if_free
 *	Free the tunnel l3 if interface
 */
static void ppe_drv_tun_l3_if_free(struct kref *kref)
{
	fal_tunnel_intf_t tun_l3_if_cfg = {0};
	struct ppe_drv_tun_l3_if *tun_l3_if;
	struct ppe_drv_pppoe *pppoe;
	struct ppe_drv_port *pp;
	sw_error_t err;

	tun_l3_if = container_of(kref, struct ppe_drv_tun_l3_if, ref);

	/*
	 * TL L3 interface instance is now free, there is no
	 * cleaning required for the instance
	 */
	err = fal_tunnel_intf_set(PPE_DRV_SWITCH_ID, tun_l3_if->index, &tun_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: port TL L3 configuration clear failed at index %u", tun_l3_if,
							tun_l3_if->index);
	}

	pp = ppe_drv_port_from_tl_l3_if(tun_l3_if);
	if (pp) {
		ppe_drv_port_tl_l3_if_detach(pp);
	} else {
		pppoe = ppe_drv_pppoe_find_session_by_tl_l3_if(tun_l3_if);
		if (pppoe) {
			ppe_drv_pppoe_tl_l3_if_detach(pppoe);
		}
	}

	if (!pp && !pppoe) {
		ppe_drv_warn("%p: neither port nor pppoe attached for tl_l3_if", tun_l3_if);
	}
}

/*
 * ppe_drv_tun_l3_if_deref
 *	Taken reference of TL L3 interface instance
 */
bool ppe_drv_tun_l3_if_deref(struct ppe_drv_tun_l3_if *tun_l3_if)
{
	uint8_t index __maybe_unused = tun_l3_if->index;
	ppe_drv_assert(kref_read(&tun_l3_if->ref), "%p: ref count under run for tun_l3_if", tun_l3_if);

	if (kref_put(&tun_l3_if->ref, ppe_drv_tun_l3_if_free)) {
		ppe_drv_trace("%p: TL L3 interface is free at index: %d", tun_l3_if, index);
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_l3_if_ref
 *	Take reference of TL L3 interface instance
 */
struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_ref(struct ppe_drv_tun_l3_if *tun_l3_if)
{
	kref_get(&tun_l3_if->ref);
	ppe_drv_assert(kref_read(&tun_l3_if->ref), "%p: ref count rollover for tun_l3_if at index:%d",
			tun_l3_if, tun_l3_if->index);
	ppe_drv_trace("%p: tun_l3_if %u ref inc:%u", tun_l3_if, tun_l3_if->index, kref_read(&tun_l3_if->ref));
	return tun_l3_if;
}

/*
 * ppe_drv_tun_l3_if_get_index
 *	Return index from tun_l3_if instance
 */
uint8_t ppe_drv_tun_l3_if_get_index(struct ppe_drv_tun_l3_if *tun_l3_if)
{
	return tun_l3_if->index;
}

/*
 * ppe_drv_tun_l3_if_configure
 *	Configure TL_L3_IF table instance
 */
bool ppe_drv_tun_l3_if_configure(struct ppe_drv_tun_l3_if *tun_l3_if)
{
	fal_tunnel_intf_t tun_l3_if_cfg = {0};
	sw_error_t err;

	ppe_drv_assert(kref_read(&tun_l3_if->ref), "%p: ref count under run for tun_l3_if", tun_l3_if);

	/*
	 * Enable decapsulation
	 */
	tun_l3_if_cfg.ipv4_decap_en = A_TRUE;
	tun_l3_if_cfg.ipv6_decap_en = A_TRUE;
	tun_l3_if_cfg.lpm_en = A_TRUE;

	/*
	 * Configure ttl exceed action to redirect the packet to CPU as of now.
	 * As there is no mechanism to flush the connection if deacce_en is set
	 * Will update this once the mechanism to handle this exception is added
	 */
	tun_l3_if_cfg.ttl_exceed_action = FAL_MAC_RDT_TO_CPU;
	tun_l3_if_cfg.ttl_exceed_deacce_en = 0;

	err = fal_tunnel_intf_set(PPE_DRV_SWITCH_ID, tun_l3_if->index, &tun_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: port TL L3 configuration failed at index %u", tun_l3_if, tun_l3_if->index);
		return false;
	}

	/*
	 * Dump TL_L3_IF Configuration
	 */
	ppe_drv_tun_l3_if_dump(tun_l3_if);

	return true;
}

/*
 * ppe_drv_tun_l3_if_alloc
 *	Return free TL L3 interface instance or requested at given index
 */
struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_alloc(struct ppe_drv *p)
{
	struct ppe_drv_tun_l3_if *pt_l3_if;
	uint16_t idx = 0;

	/*
	 * Return first free instance
	 */
	for (idx = 0; idx < PPE_DRV_TUN_L3_IF_MAX_ENTRIES; idx++) {
		pt_l3_if = &p->ptun_l3_if[idx];
		if (kref_read(&pt_l3_if->ref)) {
			continue;
		}

		kref_init(&pt_l3_if->ref);
		ppe_drv_trace("%p: Free tunnel l3 if instance found, index: %d", pt_l3_if, idx);
		return pt_l3_if;
	}

	ppe_drv_warn("%p: Free tunnel encap instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_l3_if_entries_free
 *	Free memory allocated for TL L3 interface instances.
 */
void ppe_drv_tun_l3_if_entries_free(struct ppe_drv_tun_l3_if *ptun_l3_if)
{
	vfree(ptun_l3_if);
}

/*
 * ppe_drv_tun_l3_if_entries_alloc
 *	Allocate and initialize TL L3 interface entries.
 */
struct ppe_drv_tun_l3_if *ppe_drv_tun_l3_if_entries_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_l3_if *ptun_l3_if;

	ppe_drv_assert(!p->ptun_l3_if, "%p: TL L3 interface already allocated", p);

	ptun_l3_if = vzalloc(sizeof(struct ppe_drv_tun_l3_if) * PPE_DRV_TUN_L3_IF_MAX_ENTRIES);
	if (!ptun_l3_if) {
		ppe_drv_warn("%p: failed to allocate tun_l3_if entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_L3_IF_MAX_ENTRIES; index++) {
		ptun_l3_if[index].index = index;
	}

	return ptun_l3_if;
}
