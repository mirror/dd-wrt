#include <linux/module.h>
#include <linux/time.h>
#include <linux/export.h>

#include "pdwext.h"
#include "mesh.h"
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

void tdma_originator_get_processing(struct ieee80211_if_tdma *tdma, const u8 *mac, const u8 *relay, bool * retr, bool * local)
{
	struct p_originator *o;

	*retr = false;
	*local = false;

	if (is_multicast_ether_addr(mac)) {
		*local = true;
		*retr = is_broadcast_ether_addr(relay);
	} else {
		u16 rval;

		if ((o = __tdma_originator_find_best(tdma, mac, &rval)) != NULL) {
#ifdef TDMA_DEBUG_MESH_PROCESSING
			printk("TDMA_MESH: find best route to %pM through %pM with reachable %d\n", o->mac, o->relay, rval);
#endif
			if (o->hops == 0)
				*local = true;
			else
				*retr = true;
		} else
			*retr = true;
	}
}

EXPORT_SYMBOL(tdma_originator_get_processing);

u8 tdma_originator_put(struct ieee80211_if_tdma *tdma, u8 *ptr, const u8 *mac, size_t len)
{
	u8 *p, res = 0;
	u16 rval;
	struct p_originator *tmp;
	unsigned intval = usecs_to_jiffies(tdma->tx_round_duration * TDMA_MAX_LOST_BEACON_COUNT), intval2 = usecs_to_jiffies(tdma->tx_slot_duration);

	*ptr = WLAN_EID_TDMA_MESH_INFO;
	ptr++;
	p = ptr;
	ptr++;
	if ((tmp = __tdma_originator_find_best(tdma, mac, &rval)) != NULL) {
		if (tmp->hops > 0)
			printk("TDMA_MESH: Strange error. Local addr exists as non-local\n");
	} else
		__tdma_originator_new(tdma, mac, NULL);
	tmp = tdma->ohead;
	while ((tmp != NULL) && (res < 254)) {
		if ((rval = mm_calc_reachability(tmp, intval)) > 0) {
#ifdef TDMA_DEBUG_MESH_PROPAGATE_TX
			printk("TDMA_MESH: put to beacon route to %pM through %pM with reachable %d\n", tmp->mac, tmp->relay, (int)rval);
#endif
			memcpy(ptr, tmp->mac, ETH_ALEN);
			ptr += ETH_ALEN;
			res += ETH_ALEN;
			rval = mm_update_rval(tmp, (u16)len, intval2);
			put_unaligned(htons(rval), (u16 *)ptr);
			ptr += sizeof(u16);
			res += sizeof(u16);
		}
		tmp = tmp->next;
	}
	*p = res;
	return (res + 2);
}

EXPORT_SYMBOL(tdma_originator_put);

void tdma_originator_update_tx(struct ieee80211_if_tdma *tdma, const u8 *relay, u16 duration)
{
	struct p_originator *tmp = tdma->ohead;

	if (is_multicast_ether_addr(relay))
		return;
	while (tmp != NULL) {
		if ((tmp->hops > 0) && ether_addr_equal(tmp->relay, relay)) {
			tmp->enqueued++;
#ifdef TDMA_DEBUG_MESH_TX
			printk("TDMA_MESH: busy update for route to %pM through %pM\n", tmp->mac, tmp->relay);
#endif
		}
		tmp = tmp->next;
	}
}

EXPORT_SYMBOL(tdma_originator_update_tx);

void tdma_originator_update_ack(struct ieee80211_if_tdma *tdma, const u8 *relay, bool acked, size_t len, int signal)
{
	struct p_originator *tmp = tdma->ohead;

	if (is_multicast_ether_addr(relay))
		return;
	if (acked) {
		while (tmp != NULL) {
			if ((tmp->hops > 0) && ether_addr_equal(tmp->relay, relay)) {
				mm_update_route_energy(tmp, signal);
				tmp->last_seen = jiffies;
				if (tmp->enqueued > 0)
					tmp->enqueued--;
#if defined(TDMA_DEBUG_MESH_TX) || defined(TDMA_DEBUG_MESH_RX)
				printk("TDMA_MESH: ack-based update for route to %pM through %pM\n", tmp->mac, tmp->relay);
#endif
			}
			tmp = tmp->next;
		}
	}
}

EXPORT_SYMBOL(tdma_originator_update_ack);

void tdma_originator_install_record(struct ieee80211_if_tdma *tdma, const u8 *mac, const u8 *relay, u16 reachval, int signal)
{
	struct p_originator *tmp;
	u16 rval;

	if ((tmp = __tdma_originator_find_best(tdma, mac, &rval)) != NULL) {
		if (tmp->hops == 0)
			return;
	}
	if ((tmp = __tdma_originator_find(tdma, mac, relay)) != NULL) {
		if (tmp->last_seen < jiffies) {
			mm_update_route_complete(tmp, reachval, signal);
			tmp->last_seen = jiffies;
		} else {
			mm_update_route_rval_only(tmp, reachval);
		}
#ifdef TDMA_DEBUG_MESH_PROPAGATE_RX
		printk("TDMA_MESH: updated route to %pM through %pM with reachable %d\n", mac, relay, mm_calc_reachability(tmp, usecs_to_jiffies(tdma->tx_round_duration * TDMA_MAX_LOST_BEACON_COUNT)));
#endif
	} else
		__tdma_route_new_complete(tdma, mac, relay, reachval, signal);
}

EXPORT_SYMBOL(tdma_originator_install_record);
