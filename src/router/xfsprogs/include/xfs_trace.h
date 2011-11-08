/*
 * Copyright (c) 2011 RedHat, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __TRACE_H__
#define __TRACE_H__

#define trace_xfs_alloc_exact_done(a)		((void) 0)
#define trace_xfs_alloc_exact_notfound(a)	((void) 0)
#define trace_xfs_alloc_exact_error(a)		((void) 0)
#define trace_xfs_alloc_near_nominleft(a)	((void) 0)
#define trace_xfs_alloc_near_first(a)		((void) 0)
#define trace_xfs_alloc_near_greater(a)		((void) 0)
#define trace_xfs_alloc_near_lesser(a)		((void) 0)
#define trace_xfs_alloc_near_error(a)		((void) 0)
#define trace_xfs_alloc_size_neither(a)		((void) 0)
#define trace_xfs_alloc_size_noentry(a)		((void) 0)
#define trace_xfs_alloc_size_nominleft(a)	((void) 0)
#define trace_xfs_alloc_size_done(a)		((void) 0)
#define trace_xfs_alloc_size_error(a)		((void) 0)
#define trace_xfs_alloc_small_freelist(a)	((void) 0)
#define trace_xfs_alloc_small_notenough(a)	((void) 0)
#define trace_xfs_alloc_small_done(a)		((void) 0)
#define trace_xfs_alloc_small_error(a)		((void) 0)
#define trace_xfs_alloc_vextent_badargs(a)	((void) 0)
#define trace_xfs_alloc_vextent_nofix(a)	((void) 0)
#define trace_xfs_alloc_vextent_noagbp(a)	((void) 0)
#define trace_xfs_alloc_vextent_loopfailed(a)	((void) 0)
#define trace_xfs_alloc_vextent_allfailed(a)	((void) 0)

#define trace_xfs_log_recover_item_reorder_head(a,b,c,d)	((void) 0)
#define trace_xfs_log_recover_item_reorder_tail(a,b,c,d)	((void) 0)
#define trace_xfs_log_recover_item_add_cont(a,b,c,d)	((void) 0)
#define trace_xfs_log_recover_item_add(a,b,c,d)	((void) 0)

#define trace_xfs_btree_corrupt(a,b)	((void) 0)
#define trace_xfs_da_btree_corrupt(a,b)	((void) 0)

#define trace_xfs_free_extent(a,b,c,d,e,f,g)	((void) 0)
#define trace_xfs_agf(a,b,c,d)		((void) 0)

#define trace_xfs_iext_insert(a,b,c,d,e)	((void) 0)
#define trace_xfs_iext_remove(a,b,c,d)	((void) 0)

#define trace_xfs_dir2_grow_inode(a,b)	((void) 0)
#define trace_xfs_dir2_shrink_inode(a,b)	((void) 0)

#define trace_xfs_dir2_leaf_to_node(a)	((void) 0)
#define trace_xfs_dir2_leaf_to_block(a)	((void) 0)
#define trace_xfs_dir2_leaf_addname(a)	((void) 0)
#define trace_xfs_dir2_leaf_lookup(a)	((void) 0)
#define trace_xfs_dir2_leaf_removename(a)	((void) 0)
#define trace_xfs_dir2_leaf_replace(a)	((void) 0)

#define trace_xfs_dir2_block_addname(a)	((void) 0)
#define trace_xfs_dir2_block_to_leaf(a)	((void) 0)
#define trace_xfs_dir2_block_to_sf(a)	((void) 0)
#define trace_xfs_dir2_block_lookup(a)	((void) 0)
#define trace_xfs_dir2_block_removename(a)	((void) 0)
#define trace_xfs_dir2_block_replace(a)	((void) 0)

#define trace_xfs_dir2_leafn_add(a,b)	((void) 0)
#define trace_xfs_dir2_leafn_remove(a,b)	((void) 0)
#define trace_xfs_dir2_leafn_moveents(a,b,c,d)	((void) 0)

#define trace_xfs_dir2_node_to_leaf(a)	((void) 0)
#define trace_xfs_dir2_node_addname(a)	((void) 0)
#define trace_xfs_dir2_node_lookup(a)	((void) 0)
#define trace_xfs_dir2_node_removename(a)	((void) 0)
#define trace_xfs_dir2_node_replace(a)	((void) 0)

#define trace_xfs_dir2_sf_to_block(a)	((void) 0)
#define trace_xfs_dir2_sf_addname(a)	((void) 0)
#define trace_xfs_dir2_sf_create(a)	((void) 0)
#define trace_xfs_dir2_sf_lookup(a)	((void) 0)
#define trace_xfs_dir2_sf_removename(a)	((void) 0)
#define trace_xfs_dir2_sf_replace(a)	((void) 0)
#define trace_xfs_dir2_sf_toino4(a)	((void) 0)
#define trace_xfs_dir2_sf_toino8(a)	((void) 0)

#define trace_xfs_bmap_pre_update(a,b,c,d)	((void) 0)
#define trace_xfs_bmap_post_update(a,b,c,d)	((void) 0)
#define trace_xfs_extlist(a,b,c,d)	((void) 0)
#define trace_xfs_bunmap(a,b,c,d,e)	((void) 0)

#define trace_xfs_perag_get(a,b,c,d)	((void) 0)
#define trace_xfs_perag_put(a,b,c,d)	((void) 0)

#endif /* __TRACE_H__ */
