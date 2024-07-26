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

#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include <fal/fal_pppoe.h>
#include <fal/fal_tunnel.h>
#include <fal/fal_api.h>
#include "ppe_drv.h"
#include "tun/ppe_drv_tun.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_pppoe_dump()
 *	Dumps PPPoe table configuration
 */
static void ppe_drv_pppoe_dump(struct ppe_drv_pppoe *pppoe)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_pppoe_session_t pppoe_cfg;
	sw_error_t err;

	pppoe_cfg.session_id = pppoe->session_id;
	pppoe_cfg.multi_session = A_TRUE;
	pppoe_cfg.uni_session = A_TRUE;
	pppoe_cfg.smac_valid = A_TRUE;
	pppoe_cfg.port_bitmap = 0xFF;

	err = fal_pppoe_session_table_get(PPE_DRV_SWITCH_ID, &pppoe_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: PPPoe session table query failed: %u", p, pppoe->index);
		return;
	}

	ppe_drv_trace("%p: entry_id: %u\n", pppoe, pppoe_cfg.entry_id);
	ppe_drv_trace("%p: session_id: %u\n", pppoe, pppoe_cfg.session_id);
	ppe_drv_trace("%p: multi_session: %u\n", pppoe, pppoe_cfg.multi_session);
	ppe_drv_trace("%p: uni_session: %u\n", pppoe, pppoe_cfg.uni_session);
	ppe_drv_trace("%p: vrf_id: %u\n", pppoe, pppoe_cfg.vrf_id);
	ppe_drv_trace("%p: port_bitmap: %u\n", pppoe, pppoe_cfg.port_bitmap);
	ppe_drv_trace("%p: l3_if_index: %u\n", pppoe, pppoe_cfg.l3_if_index);
	ppe_drv_trace("%p: l3_if_valid: %u\n", pppoe, pppoe_cfg.l3_if_valid);
	ppe_drv_trace("%p: tl_l3_if_index: %u\n", pppoe, pppoe_cfg.tl_l3_if_index);
	ppe_drv_trace("%p: tl_l3_if_valid: %u\n", pppoe, pppoe_cfg.tl_l3_if_valid);
	ppe_drv_trace("%p: smac_valid: %u\n", pppoe, pppoe_cfg.smac_valid);
	ppe_drv_trace("%p: mac_addr: %pM\n", pppoe, pppoe_cfg.smac_addr.uc);
}
#else
static void ppe_drv_pppoe_dump(struct ppe_drv_pppoe *pppoe)
{
}
#endif

/*
 * ppe_drv_pppoe_free()
 *	Destroys the pppoe entry in PPE.
 */
static inline void ppe_drv_pppoe_free(struct kref *kref)
{
	struct ppe_drv_pppoe *pppoe = container_of(kref, struct ppe_drv_pppoe, ref);

	ppe_drv_assert(!pppoe->l3_if, "%p: L3_IF still associated with PPPoE\n", pppoe);

	/*
	 * Update the shadow copy.
	 */
	pppoe->session_id = 0;
	memset(&pppoe->server_mac[0], 0x00, ETH_ALEN);
	ppe_drv_pppoe_dump(pppoe);
}

/*
 * ppe_drv_pppoe_ref()
 *	Take reference on pppoe.
 */
struct ppe_drv_pppoe *ppe_drv_pppoe_ref(struct ppe_drv_pppoe *pppoe)
{
	kref_get(&pppoe->ref);
	return pppoe;
}

/*
 * ppe_drv_pppoe_deref()
 *	Let go of reference on pppoe.
 */
bool ppe_drv_pppoe_deref(struct ppe_drv_pppoe *pppoe)
{
	if (kref_put(&pppoe->ref, ppe_drv_pppoe_free)) {
		ppe_drv_trace("reference goes down to 0 for pppoe: %p", pppoe);
		return true;
	}

	ppe_drv_trace("%p: pppoe: %u", pppoe, pppoe->index);

	return false;
}

#ifdef PPE_TUNNEL_ENABLE
/*
 * ppe_drv_pppoe_tl_l3_if_get
 *	Get ti_l3_if and take reference on pppoe
 */
struct ppe_drv_tun_l3_if *ppe_drv_pppoe_tl_l3_if_get(struct ppe_drv_pppoe *pppoe)
{
	if (!pppoe->tl_l3_if) {
		ppe_drv_trace("%p: No tl_l3_if found for PPPoE session %d", pppoe, pppoe->session_id);
		return NULL;
	}

	ppe_drv_pppoe_ref(pppoe);
	return ppe_drv_tun_l3_if_ref(pppoe->tl_l3_if);
}

/*
 * ppe_drv_pppoe_tl_l3_if_detach
 *	Detach tunnel L3 IF from pppoe session
 */
bool ppe_drv_pppoe_tl_l3_if_detach(struct ppe_drv_pppoe *pppoe)
{
	fal_intf_id_t pppoe_intf;
	sw_error_t err;

	if (!pppoe->tl_l3_if) {
		ppe_drv_warn("%p: tl_l3_if is already detached from pppoe", pppoe);
		return false;
	}

	memset(&pppoe_intf, 0, sizeof(fal_intf_id_t));

	err = fal_pppoe_l3_intf_set(PPE_DRV_SWITCH_ID, pppoe->index, FAL_INTF_TYPE_TUNNEL, &pppoe_intf);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Failed to clear tl_l3_if for PPPoE", pppoe);
		return false;
	}

	ppe_drv_info("%p: tl_l3_if %p detached from PPPoE", pppoe, pppoe->tl_l3_if);

	pppoe->tl_l3_if = NULL;

	ppe_drv_pppoe_deref(pppoe);

	return true;
}

/*
 * ppe_drv_pppoe_tl_l3_if_attach
 *	Attach tunnel L3 IF to pppoe session
 */
bool ppe_drv_pppoe_tl_l3_if_attach(struct ppe_drv_pppoe *pppoe, struct ppe_drv_tun_l3_if *ptun_l3_if)
{
	uint16_t tl_l3_if_idx = ptun_l3_if->index;
	fal_intf_id_t pppoe_intf;
	sw_error_t err;

	if (pppoe->tl_l3_if) {
		ppe_drv_assert(false, "%p: tl_l3_if is already attached to pppoe %p", pppoe->tl_l3_if, pppoe);
		return false;
	}

	memset(&pppoe_intf, 0, sizeof(fal_intf_id_t));

	/*
	 * Set tunnel L3 IF index in PPPoE session table.
	 */
	pppoe_intf.l3_if_index = tl_l3_if_idx;
	pppoe_intf.l3_if_valid = A_TRUE;
	err = fal_pppoe_l3_intf_set(PPE_DRV_SWITCH_ID, pppoe->index, FAL_INTF_TYPE_TUNNEL, &pppoe_intf);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Failed to set tl_l3_if %d for PPPoE", pppoe, tl_l3_if_idx);
		return false;
	}

	pppoe->tl_l3_if = ptun_l3_if;

	ppe_drv_pppoe_ref(pppoe);

	ppe_drv_info("%p: tl_l3_if %d attached to PPPoE index %d\n", pppoe, tl_l3_if_idx, pppoe->index);
	return true;
}
#endif

/*
 * ppe_drv_pppoe_l3_if_detach()
 *	Detach L3 interface from pppoe entry in PPE.
 *
 * TODO: Implement a separate API to detach tl_l3_if from pppoe session table
 */
void ppe_drv_pppoe_l3_if_detach(struct ppe_drv_pppoe *pppoe)
{
	fal_pppoe_session_t pppoe_cfg = {0};
	fal_intf_id_t cfg;

	/*
	 * Detach L3 IF attached to pppoe (if any)
	 */
	if (!pppoe->l3_if) {
		ppe_drv_warn("%p: no l3_if attached to pppoe", pppoe);
		return;
	}

	ppe_drv_trace("%p: detaching l3_if %u from pppoe %u", pppoe, pppoe->l3_if->l3_if_index, pppoe->index);

	cfg.l3_if_valid = A_FALSE;
	cfg.l3_if_index = 0;

	if (fal_pppoe_l3_intf_set(PPE_DRV_SWITCH_ID, pppoe->index, FAL_INTF_TYPE_NORMAL, &cfg) != SW_OK) {
		ppe_drv_warn("%p: error in detaching l3_if: %p", pppoe, pppoe->l3_if);
		return;
	}

	if (pppoe->is_session_added) {
		pppoe_cfg.session_id = pppoe->session_id;
		pppoe_cfg.multi_session = A_TRUE;
		pppoe_cfg.uni_session = A_TRUE;
		pppoe_cfg.smac_valid = A_TRUE;
		pppoe_cfg.port_bitmap = 0xFF;
		memcpy(&pppoe_cfg.smac_addr, &pppoe->server_mac[0], sizeof(pppoe_cfg.smac_addr));
		if (fal_pppoe_session_table_del(PPE_DRV_SWITCH_ID, &pppoe_cfg) != SW_OK) {
			ppe_drv_warn("%p: Error in clearing SESSION table index", pppoe);
			return;
		}
	}

	pppoe->is_session_added = false;
	ppe_drv_l3_if_deref(pppoe->l3_if);
	pppoe->l3_if = NULL;
	ppe_drv_pppoe_dump(pppoe);
}

/*
 * ppe_drv_pppoe_l3_if_attach()
 *	Attach L3 interface with a pppoe entry in PPE.
 *
 * TODO: Implement a separate API to attach tl_l3_if from pppoe session table
 */
void ppe_drv_pppoe_l3_if_attach(struct ppe_drv_pppoe *pppoe, struct ppe_drv_l3_if *l3_if)
{
	fal_pppoe_session_t pppoe_cfg = {0};
	fal_intf_id_t cfg;

	if (!kref_read(&pppoe->ref)) {
		ppe_drv_warn("%p: attaching l3_if to unused pppoe %u", pppoe, pppoe->index);
		return;
	}

	if (pppoe->l3_if) {
		ppe_drv_warn("%p: pppoe: %u is not attached to l3_if: %p", pppoe, pppoe->index, pppoe->l3_if);
		return;
	}

	/*
	 * Update L3_PPPOE_TABLE with corresponding L3
	 */
	ppe_drv_trace("%p: attaching l3_if %u to pppoe %u", pppoe, l3_if->l3_if_index, pppoe->index);

	cfg.l3_if_valid = A_TRUE;
	cfg.l3_if_index = l3_if->l3_if_index;

	if (fal_pppoe_l3_intf_set(PPE_DRV_SWITCH_ID, pppoe->index, FAL_INTF_TYPE_NORMAL, &cfg) != SW_OK) {
		ppe_drv_warn("%p: error in attaching l3_if: %p", pppoe, pppoe->l3_if);
		return;
	}

	pppoe_cfg.session_id = pppoe->session_id;
	pppoe_cfg.multi_session = A_TRUE;
	pppoe_cfg.uni_session = A_TRUE;
	pppoe_cfg.smac_valid = A_TRUE;
	pppoe_cfg.port_bitmap = 0xFF;
	pppoe_cfg.l3_if_index = l3_if->l3_if_index;
	pppoe_cfg.l3_if_valid = A_TRUE;

	memcpy(&pppoe_cfg.smac_addr, &pppoe->server_mac[0], sizeof(pppoe_cfg.smac_addr));

	if (fal_pppoe_session_table_add(PPE_DRV_SWITCH_ID, &pppoe_cfg) != SW_OK) {
		ppe_drv_warn("%p: Error in adding SESSION table index", pppoe);
		return;
	}

	pppoe->is_session_added = true;
	pppoe->l3_if = ppe_drv_l3_if_ref(l3_if);
	ppe_drv_pppoe_dump(pppoe);
}

/*
 * ppe_drv_pppoe_l3_if_deref()
 *	Lets go of reference to pppoe's l3_if
 */
void ppe_drv_pppoe_l3_if_deref(struct ppe_drv_pppoe *pppoe)
{
	if (!kref_read(&pppoe->ref)) {
		ppe_drv_warn("%p: operating on unused pppoe:%d", pppoe, pppoe->index);
		return;
	}

	if (!pppoe->l3_if) {
		ppe_drv_warn("%p: pppoe: %u is not attached to l3_if: %p", pppoe, pppoe->index, pppoe->l3_if);
		return;
	}

	ppe_drv_l3_if_deref(pppoe->l3_if);
}

/*
 * ppe_drv_pppoe_l3_if_get_and_ref()
 *	Returns pointer to pppoe's l3_if and takes a reference
 */
struct ppe_drv_l3_if *ppe_drv_pppoe_l3_if_get_and_ref(struct ppe_drv_pppoe *pppoe)
{
	if (!kref_read(&pppoe->ref)) {
		ppe_drv_warn("%p: operating on unused pppoe:%d", pppoe, pppoe->index);
		return NULL;
	}

	/*
	 * Take a reference on pppoe's l3_if. This will be let go when
	 * ppe_pppoe_l3_if_deref() is called.
	 */
	if (!pppoe->l3_if) {
		ppe_drv_warn("%p: pppoe: %u is not attached to l3_if: %p", pppoe, pppoe->index, pppoe->l3_if);
		return NULL;
	}

	return ppe_drv_l3_if_ref(pppoe->l3_if);
}

/*
 * ppe_drv_pppoe_find_l3_if()
 *	Find PPPoe's L3_IF
 */
struct ppe_drv_l3_if *ppe_drv_pppoe_find_l3_if(uint16_t session_id, uint8_t *smac)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pppoe *pppoe;
	uint16_t i = 0;

	for (i = 0; i < p->pppoe_session_max; i++) {

		/*
		 * Traverse through pppoe entries and return corresponding l3_if.
		 */
		pppoe = &p->pppoe[i];
		if (kref_read(&pppoe->ref) && !memcmp(pppoe->server_mac, smac, sizeof(pppoe->server_mac))
				&& (pppoe->session_id == session_id)) {
			ppe_drv_trace("%p: found pppoe entry: %p", p, pppoe);
			return pppoe->l3_if;

		}
	}

	ppe_drv_warn("%p: failed to find a valid l3_if for pppoe session_id %d mac: %pM", p, session_id, smac);

	return NULL;
}

/*
 * ppe_pppoe_find_session()
 *	Find pppoe session for given session ID and server MAC
 */
struct ppe_drv_pppoe *ppe_drv_pppoe_find_session(uint16_t session_id, uint8_t *smac)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pppoe *pppoe;
	uint16_t i = 0;

	for (i = 0; i < p->pppoe_session_max; i++) {
		pppoe = &p->pppoe[i];
		if (kref_read(&pppoe->ref) && !memcmp(pppoe->server_mac, smac, sizeof(pppoe->server_mac))
				&& (pppoe->session_id == session_id)) {
			ppe_drv_trace("%p: Found PPPoE idx(%d) for session_id(%d) and server MAC: %pM", pppoe, i, session_id, smac);
			return pppoe;
		}
	}

	ppe_drv_warn("%p: pppoe session not found for session_id %d mac: %pM", p, session_id, smac);
	return NULL;
}

#ifdef PPE_TUNNEL_ENABLE
/*
 * ppe_pppoe_find_session()
 *	Find pppoe session for given tl_l3_if
 */
struct ppe_drv_pppoe *ppe_drv_pppoe_find_session_by_tl_l3_if(struct ppe_drv_tun_l3_if *tl_l3_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pppoe *pppoe;
	uint16_t i = 0;

	for (i = 0; i < p->pppoe_session_max; i++) {
		pppoe = &p->pppoe[i];
		if (kref_read(&pppoe->ref) && (pppoe->tl_l3_if == tl_l3_if)) {
			ppe_drv_trace("%p: Found PPPoE(%p) idx(%d)", p, pppoe, i);
			return pppoe;
		}
	}

	ppe_drv_warn("%p: pppoe session not found for tl_l3_if %p", p, tl_l3_if);
	return NULL;
}
#endif

/*
 * ppe_drv_pppoe_alloc()
 *	Programs a PPPOE_SESSION entry in PPE
 */
struct ppe_drv_pppoe *ppe_drv_pppoe_alloc(uint16_t session_id, uint8_t *smac)
{
	struct ppe_drv_pppoe *pppoe = NULL, *walk;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i = 0;

	/*
	 * Get a free PPPOE
	 */
	for (i = 0; i < p->pppoe_session_max; i++) {

		/*
		 * Check if entry already exist.
		 */
		walk = &p->pppoe[i];
		if (kref_read(&walk->ref) && !memcmp(walk->server_mac, smac, sizeof(walk->server_mac))
				&& (walk->session_id == session_id)) {
			ppe_drv_warn("%p: failed to alloc, same pppoe interface exist: %p for idx(%d)", p, walk, i);
			return NULL;

		}

		/*
		 * Save first free pppoe entry to be used later
		 */
		if (!pppoe && !(kref_read(&walk->ref))) {
			pppoe = walk;
		}
	}

	if (!pppoe) {
		ppe_drv_trace("%p: failed to alloc, pppoe table full", p);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_pppoe_full);
		return NULL;
	}

	/*
	 * Take a reference on PPPOE. This will be let go by the user using the deref API
	 */
	kref_init(&pppoe->ref);

	/*
	 * Update the shadow copy.
	 */
	pppoe->session_id = session_id;
	memcpy(&pppoe->server_mac[0], &smac[0], ETH_ALEN);

	ppe_drv_pppoe_dump(pppoe);
	ppe_drv_trace("%p: pppoe %u created", pppoe, pppoe->index);
	return pppoe;
}

/*
 * ppe_drv_pppoe_entries_free()
 *	Free pppoe table entries if it was allocated.
 */
void ppe_drv_pppoe_entries_free(struct ppe_drv_pppoe *pppoe)
{
	vfree(pppoe);
}

/*
 * ppe_drv_pppoe_entries_alloc()
 *	Allocate and initialize pppoe entries.
 */
struct ppe_drv_pppoe *ppe_drv_pppoe_entries_alloc()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pppoe *pppoe;
	uint16_t i;

	pppoe = vzalloc(sizeof(struct ppe_drv_pppoe) * p->pppoe_session_max);
	if (!pppoe) {
		ppe_drv_warn("%p: failed to allocate pppoe entries", p);
		return NULL;
	}

	for (i = 0; i < p->pppoe_session_max; i++) {
		pppoe[i].index = i;
	}

	return pppoe;
}
