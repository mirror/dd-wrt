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

#include <linux/etherdevice.h>

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_l3_if_dump()
 *	Dumps L3_IF table configuration
 */
static void ppe_drv_l3_if_dump(struct ppe_drv_l3_if *l3_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_intf_macaddr_t mac_cfg = {0};
	fal_intf_entry_t l3_if_cfg = {0};
	fal_tunnel_id_t tun_cfg = {0};
	uint32_t mtu, mru;
	uint32_t mtu6, mru6;
	sw_error_t err;
	a_bool_t enable;

	err = fal_ip_intf_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: L3_IF interface query failed for l3_if index: %u err: %d", p, l3_if->l3_if_index, err);
		return;
	}

	ppe_drv_trace("%p: ttl decrement bypass en: %u\n", p, l3_if_cfg.ttl_dec_bypass_en);
	ppe_drv_trace("%p: IPv4 route en: %u\n", p, l3_if_cfg.ipv4_uc_route_en);
	ppe_drv_trace("%p: IPv6 route en: %u\n", p, l3_if_cfg.ipv6_uc_route_en);
	ppe_drv_trace("%p: ttl exceed deaccel en: %u\n", p, l3_if_cfg.ttl_exceed_deacclr_en);
	ppe_drv_trace("%p: icmp trigger en: %u\n", p, l3_if_cfg.icmp_trigger_en);
	ppe_drv_trace("%p: ttl exceed action: %u\n", p, l3_if_cfg.ttl_exceed_action);
	ppe_drv_trace("%p: mac address bitmap: %u\n", p, l3_if_cfg.mac_addr_bitmap);
	ppe_drv_trace("%p: dmac_check_en: %u\n", p, l3_if_cfg.dmac_check_en);
	ppe_drv_trace("%p: udp zero csum action: %u\n", p, l3_if_cfg.udp_zero_csum_action);
	ppe_drv_trace("%p: VPN ID: %u\n", p, l3_if_cfg.vpn_id);

	err = fal_ip_intf_mtu_mru_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mtu, &mru);
	if (err != SW_OK) {
		ppe_drv_warn("%p: ipv4 mtu/mru query failed for l3_if index: %u", p, l3_if->l3_if_index);
		return;
	}

	ppe_drv_trace("%p: IPv4 MTU : %u MRU: %u\n", p, mtu, mru);

	err = fal_ip6_intf_mtu_mru_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mtu6, &mru6);
	if (err != SW_OK) {
		ppe_drv_warn("%p: ipv6 mtu/mru query failed for l3_if index: %u", p, l3_if->l3_if_index);
		return;
	}

	ppe_drv_trace("%p: IPv6 MTU : %u MRU: %u\n", p, mtu6, mru6);

	err = fal_ip_intf_dmac_check_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &enable);
	if (err != SW_OK) {
		ppe_drv_warn("%p: DMAC check query failed for l3_if index: %u", p, l3_if->l3_if_index);
		return;
	}

	ppe_drv_trace("%p: DMAC check enabled is %u\n", p, enable);

	err = fal_ip_intf_macaddr_get_first(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: MAC address query failed for l3_if index: %u", p, l3_if->l3_if_index);
		return;
	}

	ppe_drv_trace("%p: MAC address direction is %u\n", p, mac_cfg.direction);
	ppe_drv_trace("%p: MAC address  is %pM\n", p, mac_cfg.mac_addr.uc);

	err = fal_tunnel_encap_intf_tunnelid_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &tun_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Tunnel L3_IF query failed for l3_if index: %u", p, l3_if->l3_if_index);
		return;
	}

	ppe_drv_trace("%p: tunnel l3_if is %u for l3_ifd is %u\n", p, tun_cfg.tunnel_id_valid, tun_cfg.tunnel_id);
}
#else
static void ppe_drv_l3_if_dump(struct ppe_drv_l3_if *l3_if)
{
}
#endif

/*
 * ppe_drv_l3_if_free()
 *	FREE the L3 interface entry in PPE.
 */
static void ppe_drv_l3_if_free(struct kref *kref)
{
	struct ppe_drv_l3_if *l3_if = container_of(kref, struct ppe_drv_l3_if, ref);
	fal_intf_entry_t in_l3_if_cfg = {0};
	fal_tunnel_id_t tun_cfg = {0};

	/*
	 * Clear IN_L3_IF_TBL entry.
	 */
	if (fal_ip_intf_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg) != SW_OK) {
		ppe_drv_warn("%p: Clearing L3_IF index failed for idx: %d\n", l3_if, l3_if->l3_if_index);
	}

	if (fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0) != SW_OK) {
		ppe_drv_warn("%p: Clearing IPv4 L3_IF mtu failed for idx: %d\n", l3_if, l3_if->l3_if_index);
	}

	if (fal_ip6_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0) != SW_OK) {
		ppe_drv_warn("%p: Clearing IPv6 L3_IF mtu failed for idx: %d\n", l3_if, l3_if->l3_if_index);
	}

	/*
	 * Clear IN_L3_IF_TBL and EG_L3_IF_TBL entry.
	 */
	if (!fal_tunnel_encap_intf_tunnelid_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &tun_cfg) != SW_OK) {
		ppe_drv_warn("%p: Clearing EG_L3_IF for tunnel failed\n", l3_if);
	}

	/*
	 * Clear shadow copy.
	 */
	l3_if->type = PPE_DRV_L3_IF_TYPE_MAX;
	l3_if->mtu = 0;

	ppe_drv_trace("%p: L3_IF free done for idx: %d\n", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
}

/*
 * ppe_drv_l3_if_ig_mac_addr_set()
 *	Programs the given MAC address to L3 interface in PPE ingress (MY_MAC) table
 */
static bool ppe_drv_l3_if_ig_mac_addr_set(struct ppe_drv_l3_if *l3_if, uint8_t *mac_addr)
{
	sw_error_t err;
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_intf_macaddr_t mac_cfg = {0};

	mac_cfg.direction = FAL_IP_INGRESS;
	memcpy(&mac_cfg.mac_addr, mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_add(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_my_mac_full);
		ppe_drv_warn("%p: Error in setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_ig_mac_set = true;
	ether_addr_copy(l3_if->ig_mac_addr, mac_addr);
	ppe_drv_trace("%p: setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_ig_mac_addr_clear()
 *	Clears MAC address of a given L3 interface in PPE ingress (MY_MAC) table
 */
static bool ppe_drv_l3_if_ig_mac_addr_clear(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err;
	fal_intf_macaddr_t mac_cfg = {0};

	mac_cfg.direction = FAL_IP_INGRESS;
	memcpy(&mac_cfg.mac_addr, l3_if->ig_mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_del(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in clearing mac addr for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_ig_mac_set = false;
	eth_zero_addr(l3_if->ig_mac_addr);
	ppe_drv_trace("%p: clearing mac addr of l3_if %u", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_ig_mac_add_and_ref()
 *	Add MY_MAC entry or take a reference if already added
 */
bool ppe_drv_l3_if_ig_mac_add_and_ref(struct ppe_drv_l3_if *l3_if)
{
	if (l3_if->ig_mac_ref) {
		l3_if->ig_mac_ref++;
		ppe_drv_trace("%p: ingress mac address ref: %d", l3_if, l3_if->ig_mac_ref);
		return true;
	}

	if (!l3_if->is_eg_mac_set) {
		ppe_drv_trace("%p: No egress mac address configured", l3_if);
		return false;
	}

	if (!ppe_drv_l3_if_ig_mac_addr_set(l3_if, l3_if->eg_mac_addr)) {
		ppe_drv_warn("%p: failed to set MY_MAC: %pM", l3_if, l3_if->eg_mac_addr);
		return false;
	}

	ppe_drv_trace("%p: new mac address set in MY_MAC: %pM", l3_if, l3_if->ig_mac_addr);
	l3_if->ig_mac_ref = 1;
	return true;

}

/*
 * ppe_drv_l3_if_ig_mac_deref()
 *	Release a reference on MY_MAC entry and delete the entry if reference goes down to 0
 */
bool ppe_drv_l3_if_ig_mac_deref(struct ppe_drv_l3_if *l3_if)
{
	if (--l3_if->ig_mac_ref) {
		return true;
	}

	ppe_drv_trace("%p: reference goes down to 0 for ingress mac\n", l3_if);
	if (!ppe_drv_l3_if_ig_mac_addr_clear(l3_if)) {
		return false;
	}

	return true;
}

/*
 * ppe_drv_l3_if_eg_mac_addr_set()
 *	Programs the given MAC address to L3 interface in PPE Egress table
 */
bool ppe_drv_l3_if_eg_mac_addr_set(struct ppe_drv_l3_if *l3_if, const uint8_t *mac_addr)
{
	sw_error_t err;
	fal_intf_macaddr_t mac_cfg = {0};

	mac_cfg.direction = FAL_IP_EGRESS;
	memcpy(&mac_cfg.mac_addr, mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_add(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_eg_mac_set = true;
	ether_addr_copy(l3_if->eg_mac_addr, mac_addr);
	ppe_drv_trace("%p: setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_eg_mac_addr_clear()
 *	Clears MAC address of a given L3 interface in PPE egress table
 */
bool ppe_drv_l3_if_eg_mac_addr_clear(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err;
	fal_intf_macaddr_t mac_cfg = {0};

	mac_cfg.direction = FAL_IP_EGRESS;
	memcpy(&mac_cfg.mac_addr, l3_if->eg_mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_del(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in clearing mac addr for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_eg_mac_set = false;
	eth_zero_addr(l3_if->eg_mac_addr);
	ppe_drv_trace("%p: clearing mac addr of l3_if %u", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_mac_addr_set()
 *	Programs the given MAC address to L3 interface in PPE
 */
bool ppe_drv_l3_if_mac_addr_set(struct ppe_drv_l3_if *l3_if, const uint8_t *mac_addr)
{
	sw_error_t err;
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_intf_macaddr_t mac_cfg = {0};

	/*
	 * Check if mac address is already set for given l3_if; if set then clear
	 * mac address and reconfigure the new mac address
	 */
	if (l3_if->is_mac_set) {
		mac_cfg.direction = FAL_IP_BOTH;
		memcpy(&mac_cfg.mac_addr, l3_if->ig_mac_addr, sizeof(mac_cfg.mac_addr));
		err = fal_ip_intf_macaddr_del(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
		if (err != SW_OK) {
			ppe_drv_warn("%p: Error in clearing mac addr for l3_if %u", l3_if, l3_if->l3_if_index);
		}
	}

	mac_cfg.direction = FAL_IP_BOTH;
	memcpy(&mac_cfg.mac_addr, mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_add(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_my_mac_full);
		ppe_drv_warn("%p: Error in setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_mac_set = true;
	l3_if->is_ig_mac_set = true;
	l3_if->is_eg_mac_set = true;
	ether_addr_copy(l3_if->ig_mac_addr, mac_addr);
	ether_addr_copy(l3_if->eg_mac_addr, mac_addr);

	ppe_drv_trace("%p: setting mac addr(%pM) to l3_if %u", l3_if, mac_addr, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_mac_addr_clear()
 *	Clears MAC address of a given L3 interface in PPE
 */
bool ppe_drv_l3_if_mac_addr_clear(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err;
	fal_intf_macaddr_t mac_cfg = {0};

	/*
	 * Clear mac address only if l3_if indicates that mac address was configured
	 * in PPE tables.
	 */
	if (!l3_if->is_mac_set) {
		return true;
	}

	mac_cfg.direction = FAL_IP_BOTH;
	memcpy(&mac_cfg.mac_addr, l3_if->ig_mac_addr, sizeof(mac_cfg.mac_addr));
	err = fal_ip_intf_macaddr_del(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &mac_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in clearing mac addr for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	l3_if->is_mac_set = false;
	l3_if->is_ig_mac_set = false;
	l3_if->is_eg_mac_set = false;
	eth_zero_addr(l3_if->ig_mac_addr);
	eth_zero_addr(l3_if->eg_mac_addr);

	ppe_drv_trace("%p: clearing mac addr of l3_if %u", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_mtu_mru_disable()
 *	Disable MTU/MRU to check for L3 interface in PPE
 */
bool ppe_drv_l3_if_mtu_mru_disable(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err;

	/*
	 * PPE doesn't support MTU/MRU check configuration per L3_IF.
	 * Set the MAX packet size to avoid any exception to disable MTU check for L3_IF.
	 */
	err = fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, PPE_DRV_JUMBO_MAX, PPE_DRV_JUMBO_MAX);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in setting mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	err = fal_ip6_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, PPE_DRV_JUMBO_MAX, PPE_DRV_JUMBO_MAX);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in setting IPv6 mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0);
		return false;
	}

	ppe_drv_trace("%p: disable mtu-mru on l3_if %u", l3_if, l3_if->l3_if_index);

	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_mtu_mru_set()
 *	Programs the given MTU/MRU to L3 interface in PPE
 */
bool ppe_drv_l3_if_mtu_mru_set(struct ppe_drv_l3_if *l3_if, uint16_t mtu, uint16_t mru)
{
	sw_error_t err;
	if (mtu > PPE_DRV_JUMBO_MAX || mru > PPE_DRV_JUMBO_MAX) {
		ppe_drv_warn("%p:mtu/mru out of range for l3_if %d %d\n", l3_if, mtu, mru);
		return false;
	}

	err = fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, mtu, mru);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in setting mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	err = fal_ip6_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, mtu, mru);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in setting IPv6 mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0);
		return false;
	}

	l3_if->mtu = mtu;

	ppe_drv_trace("%p: set mtu:%u mru:%u to l3_if %u", l3_if, mtu, mru, l3_if->l3_if_index);

	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_mtu_mru_clear()
 *	Clears MTU/MRU of given L3 interface in PPE
 */
bool ppe_drv_l3_if_mtu_mru_clear(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err = fal_ip_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in clearing mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	err = fal_ip6_intf_mtu_mru_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, 0, 0);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in clearing IPv6 mtu/mru for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	l3_if->mtu = 0;

	ppe_drv_trace("%p: clearing mtu/mru of l3_if %u", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_ref()
 *	Reference PPE L3 interface
 */
struct ppe_drv_l3_if *ppe_drv_l3_if_ref(struct ppe_drv_l3_if *l3_if)
{
	kref_get(&l3_if->ref);
	return l3_if;
}

/*
 * ppe_drv_l3_if_deref()
 *	Let go of reference on l3_if.
 */
bool ppe_drv_l3_if_deref(struct ppe_drv_l3_if *l3_if)
{
	if (kref_put(&l3_if->ref, ppe_drv_l3_if_free)) {
		ppe_drv_trace("%p: reference goes down to 0 for l3_if\n", l3_if);
		return true;
	}

	return false;
}

/*
 * ppe_drv_l3_if_dmac_check_set
 *	Set DMAC check configuration in L3 interface.
 */
void ppe_drv_l3_if_dmac_check_set(struct ppe_drv_l3_if *l3_if, bool enable)
{
	fal_intf_entry_t in_l3_if_cfg = {0};
	sw_error_t err;

	err = fal_ip_intf_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: DMAC check query failed for l3_if index: %u", l3_if, l3_if->l3_if_index);
		return;
	}

	in_l3_if_cfg.dmac_check_en = enable;
	if (fal_ip_intf_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg) != SW_OK) {
		ppe_drv_warn("%p: DMAC set configuration failed for state: %d\n", l3_if, enable);
		return;
	}

	ppe_drv_trace("%p: DMAC set configuration done for state: %d\n", l3_if, enable);
	ppe_drv_l3_if_dump(l3_if);
}

/*
 * ppe_drv_l3_if_disable_ttl_dec()
 *	Disables the TTL decrementing operation on the L3 interface.
 */
bool ppe_drv_l3_if_disable_ttl_dec(struct ppe_drv_l3_if *l3_if, bool disable_ttl_dec)
{
	fal_intf_entry_t in_l3_if_cfg = {0};
	sw_error_t err;

	err = fal_ip_intf_get(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: L3_IF interface query failed for l3_if index: %u err: %d\n", l3_if, l3_if->l3_if_index, err);
		return false;
	}

	in_l3_if_cfg.ttl_dec_bypass_en = disable_ttl_dec;

	err = fal_ip_intf_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: L3_IF configuration failed(%d)\n", l3_if, PPE_DRV_L3_IF_TYPE_PORT);
		return false;
	}

	return true;
}

/*
 * ppe_drv_l3_if_alloc()
 *	Allocates a free L3 interface and takes a reference.
 */
struct ppe_drv_l3_if *ppe_drv_l3_if_alloc(enum ppe_drv_l3_if_type type)
{
	struct ppe_drv_l3_if *l3_if = NULL;
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_intf_entry_t in_l3_if_cfg = { 0 };
	uint16_t i;

	/*
	 * Get a free L3 interface entry from pool
	 */
	for (i = 0; i < p->l3_if_num; i++) {
		if (!kref_read(&p->l3_if[i].ref)) {
			l3_if = &p->l3_if[i];
			break;
		}
	}

	if (!l3_if) {
		ppe_drv_warn("%p: cannot alloc L3 if, table full for type(%d)", p, type);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_l3_if_full);
		return NULL;
	}

	/*
	 * Set basic provisioning bits for L3 IF.
	 */
	in_l3_if_cfg.ttl_dec_bypass_en = A_FALSE;
	in_l3_if_cfg.ipv4_uc_route_en = A_TRUE;
	in_l3_if_cfg.ipv6_uc_route_en = A_TRUE;
	in_l3_if_cfg.ttl_exceed_deacclr_en = A_TRUE;
	in_l3_if_cfg.icmp_trigger_en = A_FALSE;
	in_l3_if_cfg.ttl_exceed_action = FAL_MAC_RDT_TO_CPU;
	in_l3_if_cfg.mac_addr_bitmap = 0;
	in_l3_if_cfg.dmac_check_en = A_TRUE;
	in_l3_if_cfg.udp_zero_csum_action = FAL_UDP_ZERO_CSUM_RECALC_MAPT;
	in_l3_if_cfg.vpn_id = 0;

	if (fal_ip_intf_set(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, &in_l3_if_cfg) != SW_OK) {
		ppe_drv_warn("%p: L3_IF configuration failed(%d)", p, type);
		return NULL;
	}

	/*
	 * Take a reference. This will be let go once the user
	 * derefs this interface.
	 */
	kref_init(&l3_if->ref);
	INIT_LIST_HEAD(&l3_if->list);
	l3_if->type = type;

	ppe_drv_l3_if_dump(l3_if);
	return l3_if;
}

/*
 * ppe_drv_l3_if_pppoe_clear()
 *	Clears pppoe session-id from the given L3 interface in PPE
 */
void ppe_drv_l3_if_pppoe_clear(struct ppe_drv_l3_if *l3_if)
{
	sw_error_t err = fal_pppoe_l3intf_enable(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, false);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Disabling pppoe failed for L3_IF %u", l3_if, l3_if->l3_if_index);
		return;
	}

	/*
	 * set session value in shadow table
	 */
	ppe_drv_pppoe_deref(l3_if->pppoe);
	l3_if->pppoe = NULL;

	ppe_drv_trace("%p: clearing pppoe on l3_if:%u", l3_if, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
}

/*
 * ppe_drv_l3_if_pppoe_set()
 *	Programs the given pppoe session-id to L3 interface in PPE
 */
bool ppe_drv_l3_if_pppoe_set(struct ppe_drv_l3_if *l3_if, struct ppe_drv_pppoe *pppoe)
{
	sw_error_t err = fal_pppoe_l3intf_enable(PPE_DRV_SWITCH_ID, l3_if->l3_if_index, true);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Enabling L3_IF failed for l3_if %u", l3_if, l3_if->l3_if_index);
		return false;
	}

	/*
	 * set session value in shadow table
	 */
	l3_if->pppoe = ppe_drv_pppoe_ref(pppoe);

	ppe_drv_trace("%p: setting pppoe:%p for l3_if:%u", l3_if, pppoe, l3_if->l3_if_index);
	ppe_drv_l3_if_dump(l3_if);
	return true;
}

/*
 * ppe_drv_l3_if_pppoe_get()
 *	Returns the pppoe associated to L3 interface in PPE
 */
struct ppe_drv_pppoe *ppe_drv_l3_if_pppoe_get(struct ppe_drv_l3_if *l3_if)
{
	return l3_if->pppoe;
}

/*
 * ppe_drv_l3_if_pppoe_match()
 *	Match pppoe session-id and smac in l3 interface
 */
bool ppe_drv_l3_if_pppoe_match(struct ppe_drv_l3_if *l3_if, uint16_t session_id, uint8_t *smac)
{
	struct ppe_drv_l3_if *pppoe_l3_if;

	if (!kref_read(&l3_if->ref)) {
		ppe_drv_warn("%p: unused l3_if %d for session_id %d", l3_if, l3_if->l3_if_index, session_id);
		return false;
	}

	pppoe_l3_if = ppe_drv_pppoe_find_l3_if(session_id, smac);
	if (!pppoe_l3_if) {
		ppe_drv_info("%p: no pppoe l3_if found corresponding to session_id: %d for mac: %pM", l3_if, session_id, smac);
		return false;
	}

	ppe_drv_trace("%p: pppoe l3_if found corresponding to session_id: %d for mac: %pM", l3_if, session_id, smac);
	return (l3_if == pppoe_l3_if);
}

/*
 * ppe_drv_l3_if_get_index
 *	Return L3_IF index
 */
uint16_t ppe_drv_l3_if_get_index(struct ppe_drv_l3_if *l3_if)
{
	return l3_if->l3_if_index;
}

/*
 * ppe_drv_l3_if_entries_free()
 *	Free l3_if table entries if it was allocated.
 */
void ppe_drv_l3_if_entries_free(struct ppe_drv_l3_if *l3_if)
{
	vfree(l3_if);
}

/*
 * ppe_drv_l3_if_entries_alloc()
 *	Allocated and initialize l3 interface entries.
 */
struct ppe_drv_l3_if *ppe_drv_l3_if_entries_alloc()
{
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	l3_if = vzalloc(sizeof(struct ppe_drv_l3_if) * p->l3_if_num);
	if (!l3_if) {
		ppe_drv_warn("%p: failed to allocate l3_if entries", p);
		return NULL;
	}

	/*
	 * Initialize interface values
	 */
	for (i = 0; i < p->l3_if_num; i++) {
		l3_if[i].l3_if_index = i;
		l3_if[i].is_mac_set = false;
	}

	return l3_if;
}
