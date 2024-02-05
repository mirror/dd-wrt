// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs_priv.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_da_format.h"
#include "xfs_trans_resv.h"
#include "xfs_bit.h"
#include "xfs_sb.h"
#include "xfs_mount.h"
#include "xfs_defer.h"
#include "xfs_trans.h"
#include "xfs_bmap.h"
#include "xfs_alloc.h"
#include "xfs_rmap.h"
#include "xfs_refcount.h"
#include "xfs_bmap.h"
#include "xfs_inode.h"
#include "xfs_da_btree.h"
#include "xfs_attr.h"
#include "libxfs.h"

/* Dummy defer item ops, since we don't do logging. */

/* Extent Freeing */

/* Sort bmap items by AG. */
static int
xfs_extent_free_diff_items(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	const struct xfs_extent_free_item *ra;
	const struct xfs_extent_free_item *rb;

	ra = container_of(a, struct xfs_extent_free_item, xefi_list);
	rb = container_of(b, struct xfs_extent_free_item, xefi_list);

	return ra->xefi_pag->pag_agno - rb->xefi_pag->pag_agno;
}

/* Get an EFI. */
static struct xfs_log_item *
xfs_extent_free_create_intent(
	struct xfs_trans		*tp,
	struct list_head		*items,
	unsigned int			count,
	bool				sort)
{
	struct xfs_mount		*mp = tp->t_mountp;

	if (sort)
		list_sort(mp, items, xfs_extent_free_diff_items);
	return NULL;
}

/* Get an EFD so we can process all the free extents. */
static struct xfs_log_item *
xfs_extent_free_create_done(
	struct xfs_trans		*tp,
	struct xfs_log_item		*intent,
	unsigned int			count)
{
	return NULL;
}

/* Take an active ref to the AG containing the space we're freeing. */
void
xfs_extent_free_get_group(
	struct xfs_mount		*mp,
	struct xfs_extent_free_item	*xefi)
{
	xfs_agnumber_t			agno;

	agno = XFS_FSB_TO_AGNO(mp, xefi->xefi_startblock);
	xefi->xefi_pag = xfs_perag_intent_get(mp, agno);
}

/* Release an active AG ref after some freeing work. */
static inline void
xfs_extent_free_put_group(
	struct xfs_extent_free_item	*xefi)
{
	xfs_perag_intent_put(xefi->xefi_pag);
}

/* Process a free extent. */
STATIC int
xfs_extent_free_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_owner_info		oinfo = { };
	struct xfs_extent_free_item	*xefi;
	xfs_agblock_t			agbno;
	int				error;

	xefi = container_of(item, struct xfs_extent_free_item, xefi_list);

	oinfo.oi_owner = xefi->xefi_owner;
	if (xefi->xefi_flags & XFS_EFI_ATTR_FORK)
		oinfo.oi_flags |= XFS_OWNER_INFO_ATTR_FORK;
	if (xefi->xefi_flags & XFS_EFI_BMBT_BLOCK)
		oinfo.oi_flags |= XFS_OWNER_INFO_BMBT_BLOCK;

	agbno = XFS_FSB_TO_AGBNO(tp->t_mountp, xefi->xefi_startblock);
	error = xfs_free_extent(tp, xefi->xefi_pag, agbno,
			xefi->xefi_blockcount, &oinfo, XFS_AG_RESV_NONE);

	xfs_extent_free_put_group(xefi);
	kmem_cache_free(xfs_extfree_item_cache, xefi);
	return error;
}

/* Abort all pending EFIs. */
STATIC void
xfs_extent_free_abort_intent(
	struct xfs_log_item		*intent)
{
}

/* Cancel a free extent. */
STATIC void
xfs_extent_free_cancel_item(
	struct list_head		*item)
{
	struct xfs_extent_free_item	*xefi;

	xefi = container_of(item, struct xfs_extent_free_item, xefi_list);

	xfs_extent_free_put_group(xefi);
	kmem_cache_free(xfs_extfree_item_cache, xefi);
}

const struct xfs_defer_op_type xfs_extent_free_defer_type = {
	.create_intent	= xfs_extent_free_create_intent,
	.abort_intent	= xfs_extent_free_abort_intent,
	.create_done	= xfs_extent_free_create_done,
	.finish_item	= xfs_extent_free_finish_item,
	.cancel_item	= xfs_extent_free_cancel_item,
};

/*
 * AGFL blocks are accounted differently in the reserve pools and are not
 * inserted into the busy extent list.
 */
STATIC int
xfs_agfl_free_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_owner_info		oinfo = { };
	struct xfs_mount		*mp = tp->t_mountp;
	struct xfs_extent_free_item	*xefi;
	struct xfs_buf			*agbp;
	int				error;
	xfs_agblock_t			agbno;

	xefi = container_of(item, struct xfs_extent_free_item, xefi_list);

	ASSERT(xefi->xefi_blockcount == 1);
	agbno = XFS_FSB_TO_AGBNO(mp, xefi->xefi_startblock);
	oinfo.oi_owner = xefi->xefi_owner;

	error = xfs_alloc_read_agf(xefi->xefi_pag, tp, 0, &agbp);
	if (!error)
		error = xfs_free_agfl_block(tp, xefi->xefi_pag->pag_agno,
				agbno, agbp, &oinfo);

	xfs_extent_free_put_group(xefi);
	kmem_cache_free(xfs_extfree_item_cache, xefi);
	return error;
}

/* sub-type with special handling for AGFL deferred frees */
const struct xfs_defer_op_type xfs_agfl_free_defer_type = {
	.create_intent	= xfs_extent_free_create_intent,
	.abort_intent	= xfs_extent_free_abort_intent,
	.create_done	= xfs_extent_free_create_done,
	.finish_item	= xfs_agfl_free_finish_item,
	.cancel_item	= xfs_extent_free_cancel_item,
};

/* Reverse Mapping */

/* Sort rmap intents by AG. */
static int
xfs_rmap_update_diff_items(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	const struct xfs_rmap_intent	*ra;
	const struct xfs_rmap_intent	*rb;

	ra = container_of(a, struct xfs_rmap_intent, ri_list);
	rb = container_of(b, struct xfs_rmap_intent, ri_list);

	return ra->ri_pag->pag_agno - rb->ri_pag->pag_agno;
}

/* Get an RUI. */
static struct xfs_log_item *
xfs_rmap_update_create_intent(
	struct xfs_trans		*tp,
	struct list_head		*items,
	unsigned int			count,
	bool				sort)
{
	 struct xfs_mount		*mp = tp->t_mountp;

	if (sort)
		list_sort(mp, items, xfs_rmap_update_diff_items);
	return NULL;
}

/* Get an RUD so we can process all the deferred rmap updates. */
static struct xfs_log_item *
xfs_rmap_update_create_done(
	struct xfs_trans		*tp,
	struct xfs_log_item		*intent,
	unsigned int			count)
{
	return NULL;
}

/* Take an active ref to the AG containing the space we're rmapping. */
void
xfs_rmap_update_get_group(
	struct xfs_mount	*mp,
	struct xfs_rmap_intent	*ri)
{
	xfs_agnumber_t	agno;

	agno = XFS_FSB_TO_AGNO(mp, ri->ri_bmap.br_startblock);
	ri->ri_pag = xfs_perag_intent_get(mp, agno);
}

/* Release an active AG ref after finishing rmapping work. */
static inline void
xfs_rmap_update_put_group(
	struct xfs_rmap_intent	*ri)
{
	xfs_perag_intent_put(ri->ri_pag);
}

/* Process a deferred rmap update. */
STATIC int
xfs_rmap_update_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_rmap_intent		*ri;
	int				error;

	ri = container_of(item, struct xfs_rmap_intent, ri_list);

	error = xfs_rmap_finish_one(tp, ri, state);

	xfs_rmap_update_put_group(ri);
	kmem_cache_free(xfs_rmap_intent_cache, ri);
	return error;
}

/* Abort all pending RUIs. */
STATIC void
xfs_rmap_update_abort_intent(
	struct xfs_log_item		*intent)
{
}

/* Cancel a deferred rmap update. */
STATIC void
xfs_rmap_update_cancel_item(
	struct list_head		*item)
{
	struct xfs_rmap_intent		*ri;

	ri = container_of(item, struct xfs_rmap_intent, ri_list);

	xfs_rmap_update_put_group(ri);
	kmem_cache_free(xfs_rmap_intent_cache, ri);
}

const struct xfs_defer_op_type xfs_rmap_update_defer_type = {
	.create_intent	= xfs_rmap_update_create_intent,
	.abort_intent	= xfs_rmap_update_abort_intent,
	.create_done	= xfs_rmap_update_create_done,
	.finish_item	= xfs_rmap_update_finish_item,
	.finish_cleanup = xfs_rmap_finish_one_cleanup,
	.cancel_item	= xfs_rmap_update_cancel_item,
};

/* Reference Counting */

/* Sort refcount intents by AG. */
static int
xfs_refcount_update_diff_items(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	const struct xfs_refcount_intent *ra;
	const struct xfs_refcount_intent *rb;

	ra = container_of(a, struct xfs_refcount_intent, ri_list);
	rb = container_of(b, struct xfs_refcount_intent, ri_list);

	return ra->ri_pag->pag_agno - rb->ri_pag->pag_agno;
}

/* Get an CUI. */
static struct xfs_log_item *
xfs_refcount_update_create_intent(
	struct xfs_trans		*tp,
	struct list_head		*items,
	unsigned int			count,
	bool				sort)
{
	struct xfs_mount		*mp = tp->t_mountp;

	if (sort)
		list_sort(mp, items, xfs_refcount_update_diff_items);
	return NULL;
}

/* Get an CUD so we can process all the deferred refcount updates. */
static struct xfs_log_item *
xfs_refcount_update_create_done(
	struct xfs_trans		*tp,
	struct xfs_log_item		*intent,
	unsigned int			count)
{
	return NULL;
}

/* Take an active ref to the AG containing the space we're refcounting. */
void
xfs_refcount_update_get_group(
	struct xfs_mount		*mp,
	struct xfs_refcount_intent	*ri)
{
	xfs_agnumber_t			agno;

	agno = XFS_FSB_TO_AGNO(mp, ri->ri_startblock);
	ri->ri_pag = xfs_perag_intent_get(mp, agno);
}

/* Release an active AG ref after finishing refcounting work. */
static inline void
xfs_refcount_update_put_group(
	struct xfs_refcount_intent	*ri)
{
	xfs_perag_intent_put(ri->ri_pag);
}

/* Process a deferred refcount update. */
STATIC int
xfs_refcount_update_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_refcount_intent	*ri;
	int				error;

	ri = container_of(item, struct xfs_refcount_intent, ri_list);
	error = xfs_refcount_finish_one(tp, ri, state);

	/* Did we run out of reservation?  Requeue what we didn't finish. */
	if (!error && ri->ri_blockcount > 0) {
		ASSERT(ri->ri_type == XFS_REFCOUNT_INCREASE ||
		       ri->ri_type == XFS_REFCOUNT_DECREASE);
		return -EAGAIN;
	}

	xfs_refcount_update_put_group(ri);
	kmem_cache_free(xfs_refcount_intent_cache, ri);
	return error;
}

/* Abort all pending CUIs. */
STATIC void
xfs_refcount_update_abort_intent(
	struct xfs_log_item		*intent)
{
}

/* Cancel a deferred refcount update. */
STATIC void
xfs_refcount_update_cancel_item(
	struct list_head		*item)
{
	struct xfs_refcount_intent	*ri;

	ri = container_of(item, struct xfs_refcount_intent, ri_list);

	xfs_refcount_update_put_group(ri);
	kmem_cache_free(xfs_refcount_intent_cache, ri);
}

const struct xfs_defer_op_type xfs_refcount_update_defer_type = {
	.create_intent	= xfs_refcount_update_create_intent,
	.abort_intent	= xfs_refcount_update_abort_intent,
	.create_done	= xfs_refcount_update_create_done,
	.finish_item	= xfs_refcount_update_finish_item,
	.finish_cleanup = xfs_refcount_finish_one_cleanup,
	.cancel_item	= xfs_refcount_update_cancel_item,
};

/* Inode Block Mapping */

/* Sort bmap intents by inode. */
static int
xfs_bmap_update_diff_items(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	const struct xfs_bmap_intent	*ba;
	const struct xfs_bmap_intent	*bb;

	ba = container_of(a, struct xfs_bmap_intent, bi_list);
	bb = container_of(b, struct xfs_bmap_intent, bi_list);
	return ba->bi_owner->i_ino - bb->bi_owner->i_ino;
}

/* Get an BUI. */
static struct xfs_log_item *
xfs_bmap_update_create_intent(
	struct xfs_trans		*tp,
	struct list_head		*items,
	unsigned int			count,
	bool				sort)
{
	struct xfs_mount		*mp = tp->t_mountp;

	if (sort)
		list_sort(mp, items, xfs_bmap_update_diff_items);
	return NULL;
}

/* Get an BUD so we can process all the deferred rmap updates. */
static struct xfs_log_item *
xfs_bmap_update_create_done(
	struct xfs_trans		*tp,
	struct xfs_log_item		*intent,
	unsigned int			count)
{
	return NULL;
}

/* Take an active ref to the AG containing the space we're mapping. */
void
xfs_bmap_update_get_group(
	struct xfs_mount	*mp,
	struct xfs_bmap_intent	*bi)
{
	xfs_agnumber_t		agno;

	agno = XFS_FSB_TO_AGNO(mp, bi->bi_bmap.br_startblock);

	/*
	 * Bump the intent count on behalf of the deferred rmap and refcount
	 * intent items that that we can queue when we finish this bmap work.
	 * This new intent item will bump the intent count before the bmap
	 * intent drops the intent count, ensuring that the intent count
	 * remains nonzero across the transaction roll.
	 */
	bi->bi_pag = xfs_perag_intent_get(mp, agno);
}

/* Release an active AG ref after finishing mapping work. */
static inline void
xfs_bmap_update_put_group(
	struct xfs_bmap_intent	*bi)
{
	xfs_perag_intent_put(bi->bi_pag);
}

/* Process a deferred rmap update. */
STATIC int
xfs_bmap_update_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_bmap_intent		*bi;
	int				error;

	bi = container_of(item, struct xfs_bmap_intent, bi_list);
	error = xfs_bmap_finish_one(tp, bi);
	if (!error && bi->bi_bmap.br_blockcount > 0) {
		ASSERT(bi->bi_type == XFS_BMAP_UNMAP);
		return -EAGAIN;
	}

	xfs_bmap_update_put_group(bi);
	kmem_cache_free(xfs_bmap_intent_cache, bi);
	return error;
}

/* Abort all pending BUIs. */
STATIC void
xfs_bmap_update_abort_intent(
	struct xfs_log_item		*intent)
{
}

/* Cancel a deferred rmap update. */
STATIC void
xfs_bmap_update_cancel_item(
	struct list_head		*item)
{
	struct xfs_bmap_intent		*bi;

	bi = container_of(item, struct xfs_bmap_intent, bi_list);

	xfs_bmap_update_put_group(bi);
	kmem_cache_free(xfs_bmap_intent_cache, bi);
}

const struct xfs_defer_op_type xfs_bmap_update_defer_type = {
	.create_intent	= xfs_bmap_update_create_intent,
	.abort_intent	= xfs_bmap_update_abort_intent,
	.create_done	= xfs_bmap_update_create_done,
	.finish_item	= xfs_bmap_update_finish_item,
	.cancel_item	= xfs_bmap_update_cancel_item,
};

/* Get an ATTRI. */
static struct xfs_log_item *
xfs_attr_create_intent(
	struct xfs_trans	*tp,
	struct list_head	*items,
	unsigned int		count,
	bool			sort)
{
	return NULL;
}

/* Abort all pending ATTRs. */
static void
xfs_attr_abort_intent(
	struct xfs_log_item	*intent)
{
}

/* Get an ATTRD so we can process all the attrs. */
static struct xfs_log_item *
xfs_attr_create_done(
	struct xfs_trans	*tp,
	struct xfs_log_item	*intent,
	unsigned int		count)
{
	return NULL;
}

static inline void
xfs_attr_free_item(
	struct xfs_attr_intent	*attr)
{
	if (attr->xattri_da_state)
		xfs_da_state_free(attr->xattri_da_state);
	if (attr->xattri_da_args->op_flags & XFS_DA_OP_RECOVERY)
		kmem_free(attr);
	else
		kmem_cache_free(xfs_attr_intent_cache, attr);
}

/* Process an attr. */
static int
xfs_attr_finish_item(
	struct xfs_trans	*tp,
	struct xfs_log_item	*done,
	struct list_head	*item,
	struct xfs_btree_cur	**state)
{
	struct xfs_attr_intent	*attr;
	int			error;
	struct xfs_da_args	*args;

	attr = container_of(item, struct xfs_attr_intent, xattri_list);
	args = attr->xattri_da_args;

	/*
	 * Always reset trans after EAGAIN cycle
	 * since the transaction is new
	 */
	args->trans = tp;

	if (XFS_TEST_ERROR(false, args->dp->i_mount, XFS_ERRTAG_LARP)) {
		error = -EIO;
		goto out;
	}

	error = xfs_attr_set_iter(attr);
	if (!error && attr->xattri_dela_state != XFS_DAS_DONE)
		error = -EAGAIN;
out:
	if (error != -EAGAIN)
		xfs_attr_free_item(attr);

	return error;
}

/* Cancel an attr */
static void
xfs_attr_cancel_item(
	struct list_head	*item)
{
	struct xfs_attr_intent	*attr;

	attr = container_of(item, struct xfs_attr_intent, xattri_list);
	xfs_attr_free_item(attr);
}

const struct xfs_defer_op_type xfs_attr_defer_type = {
	.max_items	= 1,
	.create_intent	= xfs_attr_create_intent,
	.abort_intent	= xfs_attr_abort_intent,
	.create_done	= xfs_attr_create_done,
	.finish_item	= xfs_attr_finish_item,
	.cancel_item	= xfs_attr_cancel_item,
};
