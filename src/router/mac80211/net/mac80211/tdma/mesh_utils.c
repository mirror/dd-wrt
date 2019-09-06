#include "pdwext.h"
#ifdef CPTCFG_MAC80211_TDMA_MESH
#include "mesh.h"
#endif

struct p_originator *__tdma_originator_find_best(struct ieee80211_if_tdma *tdma, const u8 *mac, u16 *rval)
{
	struct p_originator *tmp = tdma->ohead, *res = NULL;
#ifdef CPTCFG_MAC80211_TDMA_MESH
	u16 cval;
	unsigned intval = usecs_to_jiffies(tdma->tx_round_duration * TDMA_MAX_LOST_BEACON_COUNT);
#endif

	*rval = 0;
	while (tmp != NULL) {
		if (ether_addr_equal(tmp->mac, mac)) {
#ifdef CPTCFG_MAC80211_TDMA_MESH
			if (res == NULL) {
				res = tmp;
				*rval = mm_calc_reachability(tmp, intval);
			} else if ((cval = mm_calc_reachability(tmp, intval)) > *rval) {
				res = tmp;
				*rval = cval;
			}
#else
			res = tmp;
			*rval = TDMA_MAX_REACH_VAL;
			break;
#endif
		}
		tmp = tmp->next;
	}
#ifdef TDMA_DEBUG_MESH_INTERNAL
	if (res != NULL) {
		printk("TDMA_MESH: Internal. Find best route for mac %pM via %pM with reachability %d\n", mac, res->relay, *rval);
	} else
		printk("TDMA_MESH: Internal. No any route for mac %pM\n", mac);
#endif
	return (res != NULL) ? ((*rval == 0) ? NULL : res) : res;
}

struct p_originator *__tdma_originator_find(struct ieee80211_if_tdma *tdma, const u8 *mac, const u8 *relay)
{
	struct p_originator *tmp = tdma->ohead;
	u16 rval;

	if (relay == NULL)
		return __tdma_originator_find_best(tdma, mac, &rval);
	while (tmp != NULL) {
		if (ether_addr_equal(tmp->mac, mac) && ether_addr_equal(tmp->relay, relay))
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

bool __tdma_originator_new(struct ieee80211_if_tdma * tdma, const u8 *mac, const u8 *relay)
{
	bool res = false;
	struct p_originator *o;

	if ((o = kzalloc(sizeof(struct p_originator), in_interrupt()? GFP_ATOMIC : GFP_KERNEL)) != NULL) {
		memcpy(o->mac, mac, ETH_ALEN);
		if (relay != NULL)
			memcpy(o->relay, relay, ETH_ALEN);
		o->next = tdma->ohead;
		tdma->ohead = o;
		res = true;
	}
	return res;
}

bool __tdma_route_new_complete(struct ieee80211_if_tdma * tdma, const u8 *mac, const u8 *relay, u16 reachval, int signal)
{
	bool res = false;

	if ((res = __tdma_originator_new(tdma, mac, relay))) {
		if (relay != NULL) {
#ifdef CPTCFG_MAC80211_TDMA_MESH
			mm_update_route_complete(tdma->ohead, reachval, signal);
#else
			tdma->ohead->energy = (s8)signal;
#endif
		}
		tdma->ohead->last_seen = jiffies;
#ifdef TDMA_DEBUG_MESH_PROPAGATE_RX
		printk("TDMA_MESH: installed new route to %pM through %pM with signal %d\n", mac, relay, signal);
#endif
	}
	return res;
}
