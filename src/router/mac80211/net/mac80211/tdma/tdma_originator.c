#include <linux/module.h>
#include <linux/time.h>
#include <linux/export.h>

#include "pdwext.h"
#include "mesh.h"
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

bool tdma_originator_update_rx(struct ieee80211_if_tdma *tdma, const u8 *mac, const u8 *relay, size_t len, int signal)
{
	struct p_originator *tmp = tdma->ohead;
	bool res = false;

	if (is_multicast_ether_addr(mac) || ((relay != NULL) && is_broadcast_ether_addr(relay)))
		return res;

	if (relay != NULL) {
		while (tmp != NULL) {
			if ((tmp->hops > 0) && ether_addr_equal(tmp->relay, relay)) {
				if (ether_addr_equal(tmp->mac, mac)) {
#ifdef CPTCFG_MAC80211_TDMA_MESH
#endif
					res = true;
				}
#ifdef CPTCFG_MAC80211_TDMA_MESH
				mm_update_route_energy(tmp, signal);
#endif
				tmp->last_seen = jiffies;
#ifdef TDMA_DEBUG_MESH_RX
				printk("TDMA_MESH: rx-based update for route to %pM through %pM\n", tmp->mac, tmp->relay);
#endif
			}
			tmp = tmp->next;
		}
	} else {
		u16 rval;

		if ((tmp = __tdma_originator_find_best(tdma, mac, &rval)) != NULL)
			res = ! !(tmp->hops == 0);
	}
	if (!res)
		res = __tdma_route_new_complete(tdma, mac, relay, 0, signal);
	return res;
}

EXPORT_SYMBOL(tdma_originator_update_rx);

bool tdma_originator_get(struct ieee80211_if_tdma * tdma, const u8 *mac, u8 *relay)
{
	struct p_originator *o;
	bool broadcast = true;
#ifdef TDMA_DEBUG_MESH_ROUTE
	printk("TDMA_MESH: Try to find route to %pM\n", mac);
#endif

	eth_broadcast_addr(relay);
	if (!is_multicast_ether_addr(mac)) {
		u16 rval;

		if ((o = __tdma_originator_find_best(tdma, mac, &rval)) != NULL) {
#ifdef TDMA_DEBUG_MESH_ROUTE
			printk("TDMA_MESH: find route to %pM through %pM with reachability %d\n", o->mac, o->relay, rval);
#endif
			if (o->hops > 0) {
				memcpy(relay, o->relay, ETH_ALEN);
				broadcast = false;
			} else
				printk("TDMA_MESH: ERROR - find local route\n");
#if defined(CPTCFG_MAC80211_DEBUGFS) && defined(CPTCFG_MAC80211_TDMA_MESH)
			tdma_mesh_store_path(tdma, o, rval);
#endif
		}
	}
	return broadcast;
}

EXPORT_SYMBOL(tdma_originator_get);

int tdma_originator_expire(struct ieee80211_if_tdma *tdma, long timeout)
{
	int expired = 0;
	struct p_originator *o = tdma->ohead;

	while (o) {
		struct p_originator *tmp = o->next;

		if (timeout == -1L) {
#ifdef TDMA_DEBUG_MESH_EXPIRATION
			printk("TDMA_MESH: expire route to %pM through %pM with reachable\n", o->mac, o->relay);
#endif
			expired++;
			tdma->ohead = tmp;
			kfree(o);
		}
		o = tmp;
	}
	return expired;
}

EXPORT_SYMBOL(tdma_originator_expire);
