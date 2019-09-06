/*
 * Some TDMA MESH support code for Linux wireless stack.
 *
 * Copyright 2011-2015	Stanislav V. Korsakov <sta@stasoft.net>
 */
#ifndef _PNMESH_H
#define _PNMESH_H

#define TDMA_MESH_GET_HOPS(val)	(((val) & 0xFF00) >> 8)
#define TDMA_MESH_PACK_HOPS(val, hops)	(((val) & 0xFF) | (((hops) & 0xFF) << 8))
#define TDMA_MESH_GET_ENQUEUED(val)	((val) & 0xFF)
#define TDMA_MESH_HOP_PENALTY	0x842
#define TDMA_MESH_ENQ_PENALTY	0x214
#define TDMA_MESH_ENERGY_SCALE(val)	(((u16)((val) << 3)) - (u16)val)
#define TDMA_MESH_CHECK_PENALTY(a, b)	(((u32)a > (u32)b) ? (u32)b : (u32)a)

extern void mm_update_route_complete(struct p_originator *, u16, int);
extern void mm_update_route_energy(struct p_originator *, int);
extern u16 mm_update_rval(struct p_originator *, size_t, unsigned);
extern void mm_update_route_rval_only(struct p_originator *, u16);
extern u16 mm_calc_reachability(struct p_originator *, unsigned);

extern struct p_originator *__tdma_originator_find_best(struct ieee80211_if_tdma *, const u8 *, u16 *);
extern struct p_originator *__tdma_originator_find(struct ieee80211_if_tdma *, const u8 *, const u8 *);
extern bool __tdma_originator_new(struct ieee80211_if_tdma *, const u8 *, const u8 *);
extern bool __tdma_route_new_complete(struct ieee80211_if_tdma *, const u8 *, const u8 *, u16, int);

#endif
