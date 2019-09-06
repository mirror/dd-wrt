#include "pnwext.h"
#include "mesh.h"

#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(a)

#define TDMA_MESH_ENERGY_TIME_INTERVAL	5000
#define TDMA_MESH_ENERGY_POWER_INTERVAL	5
#define TDMA_MESH_ENERGY_POWER_INTERVAL2	15
#define TDMA_MESH_ENERGY_POWER_INTERVAL3	25
#define TDMA_MESH_ENERGY_MEDIAN	41

static s8 mm_update_energy_first(int signal)
{
	if (signal != 13) {

		if (signal > 0)
			signal = -signal;
		return (s8)(signal + TDMA_MESH_ENERGY_MEDIAN);
	}
	return 0;
}

void mm_update_route_rval_only(struct p_originator *o, u16 rval)
{
	o->hops = TDMA_MESH_GET_HOPS(rval) + 1;
	o->enqueued = TDMA_MESH_GET_ENQUEUED(rval);
}

EXPORT_SYMBOL(mm_update_route_rval_only);

void mm_update_route_energy(struct p_originator *o, int signal)
{

	o->energy = mm_update_energy_first(signal);
}

EXPORT_SYMBOL(mm_update_route_energy);

void mm_update_route_complete(struct p_originator *o, u16 rval, int signal)
{

	o->energy = mm_update_energy_first(signal);
	mm_update_route_rval_only(o, rval);
}

EXPORT_SYMBOL(mm_update_route_complete);

u16 mm_update_rval(struct p_originator *o, size_t len, unsigned intval)
{
	u16 rval = (u16)len;
	u16 hops = o->hops;

	if ((o->hops > 0) && (jiffies > o->last_seen))
		rval += (u16)((jiffies - o->last_seen) / intval);
	rval = (u16)rval & 0xFF;
	if (o->energy < 0) {
		if (o->energy < -TDMA_MESH_ENERGY_POWER_INTERVAL3)
			hops += 4;
		else if (o->energy < -TDMA_MESH_ENERGY_POWER_INTERVAL2)
			hops += 2;
		else if (o->energy < -TDMA_MESH_ENERGY_POWER_INTERVAL)
			hops += 1;
	}
	rval = TDMA_MESH_PACK_HOPS(rval, hops);
	return rval;
}

EXPORT_SYMBOL(mm_update_rval);

u16 mm_calc_reachability(struct p_originator *o, unsigned intval)
{
	u16 reachval = TDMA_MAX_REACH_VAL;
	u32 penalty;

	penalty = o->hops * TDMA_MESH_HOP_PENALTY;
	penalty = TDMA_MESH_CHECK_PENALTY(penalty, reachval);
	reachval -= (u16)penalty;

	penalty = o->enqueued * TDMA_MESH_ENQ_PENALTY;
	penalty = TDMA_MESH_CHECK_PENALTY(penalty, reachval);
	reachval -= (u16)penalty;

	if (o->energy < 0)
		penalty = -o->energy;
	else
		penalty = o->energy;
	penalty = TDMA_MESH_ENERGY_SCALE(penalty);
	penalty *= penalty;
	penalty = TDMA_MESH_CHECK_PENALTY(penalty, reachval);
	reachval -= (u16)penalty;

	if ((o->hops > 0) && (jiffies > o->last_seen)) {
		penalty = ((jiffies - o->last_seen) * TDMA_MAX_REACH_VAL) / intval;
		penalty = TDMA_MESH_CHECK_PENALTY(penalty, reachval);
		reachval -= (u16)penalty;
	}

	return reachval;
}

EXPORT_SYMBOL(mm_calc_reachability);
