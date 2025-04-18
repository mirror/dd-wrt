// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_bit.h"
#include "xfs_shared.h"
#include "xfs_mount.h"
#include "xfs_ag.h"
#include "xfs_defer.h"
#include "xfs_trans.h"
#include "xfs_trans_priv.h"
#include "xfs_extfree_item.h"
#include "xfs_log.h"
#include "xfs_btree.h"
#include "xfs_rmap.h"
#include "xfs_alloc.h"
#include "xfs_bmap.h"
#include "xfs_trace.h"
#include "xfs_error.h"
#include "xfs_log_priv.h"
#include "xfs_log_recover.h"

struct kmem_cache	*xfs_efi_cache;
struct kmem_cache	*xfs_efd_cache;

static const struct xfs_item_ops xfs_efi_item_ops;

static inline struct xfs_efi_log_item *EFI_ITEM(struct xfs_log_item *lip)
{
	return container_of(lip, struct xfs_efi_log_item, efi_item);
}

STATIC void
xfs_efi_item_free(
	struct xfs_efi_log_item	*efip)
{
	kmem_free(efip->efi_item.li_lv_shadow);
	if (efip->efi_format.efi_nextents > XFS_EFI_MAX_FAST_EXTENTS)
		kmem_free(efip);
	else
		kmem_cache_free(xfs_efi_cache, efip);
}

/*
 * Freeing the efi requires that we remove it from the AIL if it has already
 * been placed there. However, the EFI may not yet have been placed in the AIL
 * when called by xfs_efi_release() from EFD processing due to the ordering of
 * committed vs unpin operations in bulk insert operations. Hence the reference
 * count to ensure only the last caller frees the EFI.
 */
STATIC void
xfs_efi_release(
	struct xfs_efi_log_item	*efip)
{
	ASSERT(atomic_read(&efip->efi_refcount) > 0);
	if (!atomic_dec_and_test(&efip->efi_refcount))
		return;

	xfs_trans_ail_delete(&efip->efi_item, 0);
	xfs_efi_item_free(efip);
}

STATIC void
xfs_efi_item_size(
	struct xfs_log_item	*lip,
	int			*nvecs,
	int			*nbytes)
{
	struct xfs_efi_log_item	*efip = EFI_ITEM(lip);

	*nvecs += 1;
	*nbytes += xfs_efi_log_format_sizeof(efip->efi_format.efi_nextents);
}

/*
 * This is called to fill in the vector of log iovecs for the
 * given efi log item. We use only 1 iovec, and we point that
 * at the efi_log_format structure embedded in the efi item.
 * It is at this point that we assert that all of the extent
 * slots in the efi item have been filled.
 */
STATIC void
xfs_efi_item_format(
	struct xfs_log_item	*lip,
	struct xfs_log_vec	*lv)
{
	struct xfs_efi_log_item	*efip = EFI_ITEM(lip);
	struct xfs_log_iovec	*vecp = NULL;

	ASSERT(atomic_read(&efip->efi_next_extent) ==
				efip->efi_format.efi_nextents);

	efip->efi_format.efi_type = XFS_LI_EFI;
	efip->efi_format.efi_size = 1;

	xlog_copy_iovec(lv, &vecp, XLOG_REG_TYPE_EFI_FORMAT,
			&efip->efi_format,
			xfs_efi_log_format_sizeof(efip->efi_format.efi_nextents));
}


/*
 * The unpin operation is the last place an EFI is manipulated in the log. It is
 * either inserted in the AIL or aborted in the event of a log I/O error. In
 * either case, the EFI transaction has been successfully committed to make it
 * this far. Therefore, we expect whoever committed the EFI to either construct
 * and commit the EFD or drop the EFD's reference in the event of error. Simply
 * drop the log's EFI reference now that the log is done with it.
 */
STATIC void
xfs_efi_item_unpin(
	struct xfs_log_item	*lip,
	int			remove)
{
	struct xfs_efi_log_item	*efip = EFI_ITEM(lip);
	xfs_efi_release(efip);
}

/*
 * The EFI has been either committed or aborted if the transaction has been
 * cancelled. If the transaction was cancelled, an EFD isn't going to be
 * constructed and thus we free the EFI here directly.
 */
STATIC void
xfs_efi_item_release(
	struct xfs_log_item	*lip)
{
	xfs_efi_release(EFI_ITEM(lip));
}

/*
 * Allocate and initialize an efi item with the given number of extents.
 */
STATIC struct xfs_efi_log_item *
xfs_efi_init(
	struct xfs_mount	*mp,
	uint			nextents)

{
	struct xfs_efi_log_item	*efip;

	ASSERT(nextents > 0);
	if (nextents > XFS_EFI_MAX_FAST_EXTENTS) {
		efip = kzalloc(xfs_efi_log_item_sizeof(nextents),
				GFP_KERNEL | __GFP_NOFAIL);
	} else {
		efip = kmem_cache_zalloc(xfs_efi_cache,
					 GFP_KERNEL | __GFP_NOFAIL);
	}

	xfs_log_item_init(mp, &efip->efi_item, XFS_LI_EFI, &xfs_efi_item_ops);
	efip->efi_format.efi_nextents = nextents;
	efip->efi_format.efi_id = (uintptr_t)(void *)efip;
	atomic_set(&efip->efi_next_extent, 0);
	atomic_set(&efip->efi_refcount, 2);

	return efip;
}

/*
 * Copy an EFI format buffer from the given buf, and into the destination
 * EFI format structure.
 * The given buffer can be in 32 bit or 64 bit form (which has different padding),
 * one of which will be the native format for this kernel.
 * It will handle the conversion of formats if necessary.
 */
STATIC int
xfs_efi_copy_format(xfs_log_iovec_t *buf, xfs_efi_log_format_t *dst_efi_fmt)
{
	xfs_efi_log_format_t *src_efi_fmt = buf->i_addr;
	uint i;
	uint len = xfs_efi_log_format_sizeof(src_efi_fmt->efi_nextents);
	uint len32 = xfs_efi_log_format32_sizeof(src_efi_fmt->efi_nextents);
	uint len64 = xfs_efi_log_format64_sizeof(src_efi_fmt->efi_nextents);

	if (buf->i_len == len) {
		memcpy(dst_efi_fmt, src_efi_fmt,
		       offsetof(struct xfs_efi_log_format, efi_extents));
		for (i = 0; i < src_efi_fmt->efi_nextents; i++)
			memcpy(&dst_efi_fmt->efi_extents[i],
			       &src_efi_fmt->efi_extents[i],
			       sizeof(struct xfs_extent));
		return 0;
	} else if (buf->i_len == len32) {
		xfs_efi_log_format_32_t *src_efi_fmt_32 = buf->i_addr;

		dst_efi_fmt->efi_type     = src_efi_fmt_32->efi_type;
		dst_efi_fmt->efi_size     = src_efi_fmt_32->efi_size;
		dst_efi_fmt->efi_nextents = src_efi_fmt_32->efi_nextents;
		dst_efi_fmt->efi_id       = src_efi_fmt_32->efi_id;
		for (i = 0; i < dst_efi_fmt->efi_nextents; i++) {
			dst_efi_fmt->efi_extents[i].ext_start =
				src_efi_fmt_32->efi_extents[i].ext_start;
			dst_efi_fmt->efi_extents[i].ext_len =
				src_efi_fmt_32->efi_extents[i].ext_len;
		}
		return 0;
	} else if (buf->i_len == len64) {
		xfs_efi_log_format_64_t *src_efi_fmt_64 = buf->i_addr;

		dst_efi_fmt->efi_type     = src_efi_fmt_64->efi_type;
		dst_efi_fmt->efi_size     = src_efi_fmt_64->efi_size;
		dst_efi_fmt->efi_nextents = src_efi_fmt_64->efi_nextents;
		dst_efi_fmt->efi_id       = src_efi_fmt_64->efi_id;
		for (i = 0; i < dst_efi_fmt->efi_nextents; i++) {
			dst_efi_fmt->efi_extents[i].ext_start =
				src_efi_fmt_64->efi_extents[i].ext_start;
			dst_efi_fmt->efi_extents[i].ext_len =
				src_efi_fmt_64->efi_extents[i].ext_len;
		}
		return 0;
	}
	XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, NULL, buf->i_addr,
			buf->i_len);
	return -EFSCORRUPTED;
}

static inline struct xfs_efd_log_item *EFD_ITEM(struct xfs_log_item *lip)
{
	return container_of(lip, struct xfs_efd_log_item, efd_item);
}

STATIC void
xfs_efd_item_free(struct xfs_efd_log_item *efdp)
{
	kmem_free(efdp->efd_item.li_lv_shadow);
	if (efdp->efd_format.efd_nextents > XFS_EFD_MAX_FAST_EXTENTS)
		kmem_free(efdp);
	else
		kmem_cache_free(xfs_efd_cache, efdp);
}

STATIC void
xfs_efd_item_size(
	struct xfs_log_item	*lip,
	int			*nvecs,
	int			*nbytes)
{
	struct xfs_efd_log_item	*efdp = EFD_ITEM(lip);

	*nvecs += 1;
	*nbytes += xfs_efd_log_format_sizeof(efdp->efd_format.efd_nextents);
}

/*
 * This is called to fill in the vector of log iovecs for the
 * given efd log item. We use only 1 iovec, and we point that
 * at the efd_log_format structure embedded in the efd item.
 * It is at this point that we assert that all of the extent
 * slots in the efd item have been filled.
 */
STATIC void
xfs_efd_item_format(
	struct xfs_log_item	*lip,
	struct xfs_log_vec	*lv)
{
	struct xfs_efd_log_item	*efdp = EFD_ITEM(lip);
	struct xfs_log_iovec	*vecp = NULL;

	ASSERT(efdp->efd_next_extent == efdp->efd_format.efd_nextents);

	efdp->efd_format.efd_type = XFS_LI_EFD;
	efdp->efd_format.efd_size = 1;

	xlog_copy_iovec(lv, &vecp, XLOG_REG_TYPE_EFD_FORMAT,
			&efdp->efd_format,
			xfs_efd_log_format_sizeof(efdp->efd_format.efd_nextents));
}

/*
 * The EFD is either committed or aborted if the transaction is cancelled. If
 * the transaction is cancelled, drop our reference to the EFI and free the EFD.
 */
STATIC void
xfs_efd_item_release(
	struct xfs_log_item	*lip)
{
	struct xfs_efd_log_item	*efdp = EFD_ITEM(lip);

	xfs_efi_release(efdp->efd_efip);
	xfs_efd_item_free(efdp);
}

static struct xfs_log_item *
xfs_efd_item_intent(
	struct xfs_log_item	*lip)
{
	return &EFD_ITEM(lip)->efd_efip->efi_item;
}

static const struct xfs_item_ops xfs_efd_item_ops = {
	.flags		= XFS_ITEM_RELEASE_WHEN_COMMITTED |
			  XFS_ITEM_INTENT_DONE,
	.iop_size	= xfs_efd_item_size,
	.iop_format	= xfs_efd_item_format,
	.iop_release	= xfs_efd_item_release,
	.iop_intent	= xfs_efd_item_intent,
};

/*
 * Allocate an "extent free done" log item that will hold nextents worth of
 * extents.  The caller must use all nextents extents, because we are not
 * flexible about this at all.
 */
static struct xfs_efd_log_item *
xfs_trans_get_efd(
	struct xfs_trans		*tp,
	struct xfs_efi_log_item		*efip,
	unsigned int			nextents)
{
	struct xfs_efd_log_item		*efdp;

	ASSERT(nextents > 0);

	if (nextents > XFS_EFD_MAX_FAST_EXTENTS) {
		efdp = kzalloc(xfs_efd_log_item_sizeof(nextents),
				GFP_KERNEL | __GFP_NOFAIL);
	} else {
		efdp = kmem_cache_zalloc(xfs_efd_cache,
					GFP_KERNEL | __GFP_NOFAIL);
	}

	xfs_log_item_init(tp->t_mountp, &efdp->efd_item, XFS_LI_EFD,
			  &xfs_efd_item_ops);
	efdp->efd_efip = efip;
	efdp->efd_format.efd_nextents = nextents;
	efdp->efd_format.efd_efi_id = efip->efi_format.efi_id;

	xfs_trans_add_item(tp, &efdp->efd_item);
	return efdp;
}

/*
 * Free an extent and log it to the EFD. Note that the transaction is marked
 * dirty regardless of whether the extent free succeeds or fails to support the
 * EFI/EFD lifecycle rules.
 */
static int
xfs_trans_free_extent(
	struct xfs_trans		*tp,
	struct xfs_efd_log_item		*efdp,
	struct xfs_extent_free_item	*xefi)
{
	struct xfs_owner_info		oinfo = { };
	struct xfs_mount		*mp = tp->t_mountp;
	struct xfs_extent		*extp;
	uint				next_extent;
	xfs_agblock_t			agbno = XFS_FSB_TO_AGBNO(mp,
							xefi->xefi_startblock);
	int				error;

	oinfo.oi_owner = xefi->xefi_owner;
	if (xefi->xefi_flags & XFS_EFI_ATTR_FORK)
		oinfo.oi_flags |= XFS_OWNER_INFO_ATTR_FORK;
	if (xefi->xefi_flags & XFS_EFI_BMBT_BLOCK)
		oinfo.oi_flags |= XFS_OWNER_INFO_BMBT_BLOCK;

	trace_xfs_bmap_free_deferred(tp->t_mountp, xefi->xefi_pag->pag_agno, 0,
			agbno, xefi->xefi_blockcount);

	error = __xfs_free_extent(tp, xefi->xefi_pag, agbno,
			xefi->xefi_blockcount, &oinfo, xefi->xefi_agresv,
			xefi->xefi_flags & XFS_EFI_SKIP_DISCARD);

	/*
	 * Mark the transaction dirty, even on error. This ensures the
	 * transaction is aborted, which:
	 *
	 * 1.) releases the EFI and frees the EFD
	 * 2.) shuts down the filesystem
	 */
	tp->t_flags |= XFS_TRANS_DIRTY | XFS_TRANS_HAS_INTENT_DONE;
	set_bit(XFS_LI_DIRTY, &efdp->efd_item.li_flags);

	next_extent = efdp->efd_next_extent;
	ASSERT(next_extent < efdp->efd_format.efd_nextents);
	extp = &(efdp->efd_format.efd_extents[next_extent]);
	extp->ext_start = xefi->xefi_startblock;
	extp->ext_len = xefi->xefi_blockcount;
	efdp->efd_next_extent++;

	return error;
}

/* Sort bmap items by AG. */
static int
xfs_extent_free_diff_items(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	struct xfs_extent_free_item	*ra;
	struct xfs_extent_free_item	*rb;

	ra = container_of(a, struct xfs_extent_free_item, xefi_list);
	rb = container_of(b, struct xfs_extent_free_item, xefi_list);

	return ra->xefi_pag->pag_agno - rb->xefi_pag->pag_agno;
}

/* Log a free extent to the intent item. */
STATIC void
xfs_extent_free_log_item(
	struct xfs_trans		*tp,
	struct xfs_efi_log_item		*efip,
	struct xfs_extent_free_item	*xefi)
{
	uint				next_extent;
	struct xfs_extent		*extp;

	tp->t_flags |= XFS_TRANS_DIRTY;
	set_bit(XFS_LI_DIRTY, &efip->efi_item.li_flags);

	/*
	 * atomic_inc_return gives us the value after the increment;
	 * we want to use it as an array index so we need to subtract 1 from
	 * it.
	 */
	next_extent = atomic_inc_return(&efip->efi_next_extent) - 1;
	ASSERT(next_extent < efip->efi_format.efi_nextents);
	extp = &efip->efi_format.efi_extents[next_extent];
	extp->ext_start = xefi->xefi_startblock;
	extp->ext_len = xefi->xefi_blockcount;
}

static struct xfs_log_item *
xfs_extent_free_create_intent(
	struct xfs_trans		*tp,
	struct list_head		*items,
	unsigned int			count,
	bool				sort)
{
	struct xfs_mount		*mp = tp->t_mountp;
	struct xfs_efi_log_item		*efip = xfs_efi_init(mp, count);
	struct xfs_extent_free_item	*xefi;

	ASSERT(count > 0);

	xfs_trans_add_item(tp, &efip->efi_item);
	if (sort)
		list_sort(mp, items, xfs_extent_free_diff_items);
	list_for_each_entry(xefi, items, xefi_list)
		xfs_extent_free_log_item(tp, efip, xefi);
	return &efip->efi_item;
}

/* Get an EFD so we can process all the free extents. */
static struct xfs_log_item *
xfs_extent_free_create_done(
	struct xfs_trans		*tp,
	struct xfs_log_item		*intent,
	unsigned int			count)
{
	return &xfs_trans_get_efd(tp, EFI_ITEM(intent), count)->efd_item;
}

/* Take a passive ref to the AG containing the space we're freeing. */
void
xfs_extent_free_get_group(
	struct xfs_mount		*mp,
	struct xfs_extent_free_item	*xefi)
{
	xfs_agnumber_t			agno;

	agno = XFS_FSB_TO_AGNO(mp, xefi->xefi_startblock);
	xefi->xefi_pag = xfs_perag_get(mp, agno);
}

/* Release a passive AG ref after some freeing work. */
static inline void
xfs_extent_free_put_group(
	struct xfs_extent_free_item	*xefi)
{
	xfs_perag_put(xefi->xefi_pag);
}

/* Process a free extent. */
STATIC int
xfs_extent_free_finish_item(
	struct xfs_trans		*tp,
	struct xfs_log_item		*done,
	struct list_head		*item,
	struct xfs_btree_cur		**state)
{
	struct xfs_extent_free_item	*xefi;
	int				error;

	xefi = container_of(item, struct xfs_extent_free_item, xefi_list);

	error = xfs_trans_free_extent(tp, EFD_ITEM(done), xefi);

	xfs_extent_free_put_group(xefi);
	kmem_cache_free(xfs_extfree_item_cache, xefi);
	return error;
}

/* Abort all pending EFIs. */
STATIC void
xfs_extent_free_abort_intent(
	struct xfs_log_item		*intent)
{
	xfs_efi_release(EFI_ITEM(intent));
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
	.max_items	= XFS_EFI_MAX_FAST_EXTENTS,
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
	struct xfs_efd_log_item		*efdp = EFD_ITEM(done);
	struct xfs_extent_free_item	*xefi;
	struct xfs_extent		*extp;
	struct xfs_buf			*agbp;
	int				error;
	xfs_agblock_t			agbno;
	uint				next_extent;

	xefi = container_of(item, struct xfs_extent_free_item, xefi_list);
	ASSERT(xefi->xefi_blockcount == 1);
	agbno = XFS_FSB_TO_AGBNO(mp, xefi->xefi_startblock);
	oinfo.oi_owner = xefi->xefi_owner;

	trace_xfs_agfl_free_deferred(mp, xefi->xefi_pag->pag_agno, 0, agbno,
			xefi->xefi_blockcount);

	error = xfs_alloc_read_agf(xefi->xefi_pag, tp, 0, &agbp);
	if (!error)
		error = xfs_free_agfl_block(tp, xefi->xefi_pag->pag_agno,
				agbno, agbp, &oinfo);

	/*
	 * Mark the transaction dirty, even on error. This ensures the
	 * transaction is aborted, which:
	 *
	 * 1.) releases the EFI and frees the EFD
	 * 2.) shuts down the filesystem
	 */
	tp->t_flags |= XFS_TRANS_DIRTY;
	set_bit(XFS_LI_DIRTY, &efdp->efd_item.li_flags);

	next_extent = efdp->efd_next_extent;
	ASSERT(next_extent < efdp->efd_format.efd_nextents);
	extp = &(efdp->efd_format.efd_extents[next_extent]);
	extp->ext_start = xefi->xefi_startblock;
	extp->ext_len = xefi->xefi_blockcount;
	efdp->efd_next_extent++;

	xfs_extent_free_put_group(xefi);
	kmem_cache_free(xfs_extfree_item_cache, xefi);
	return error;
}

/* sub-type with special handling for AGFL deferred frees */
const struct xfs_defer_op_type xfs_agfl_free_defer_type = {
	.max_items	= XFS_EFI_MAX_FAST_EXTENTS,
	.create_intent	= xfs_extent_free_create_intent,
	.abort_intent	= xfs_extent_free_abort_intent,
	.create_done	= xfs_extent_free_create_done,
	.finish_item	= xfs_agfl_free_finish_item,
	.cancel_item	= xfs_extent_free_cancel_item,
};

/* Is this recovered EFI ok? */
static inline bool
xfs_efi_validate_ext(
	struct xfs_mount		*mp,
	struct xfs_extent		*extp)
{
	return xfs_verify_fsbext(mp, extp->ext_start, extp->ext_len);
}

/*
 * Process an extent free intent item that was recovered from
 * the log.  We need to free the extents that it describes.
 */
STATIC int
xfs_efi_item_recover(
	struct xfs_log_item		*lip,
	struct list_head		*capture_list)
{
	struct xfs_trans_res		resv;
	struct xfs_efi_log_item		*efip = EFI_ITEM(lip);
	struct xfs_mount		*mp = lip->li_log->l_mp;
	struct xfs_efd_log_item		*efdp;
	struct xfs_trans		*tp;
	int				i;
	int				error = 0;

	/*
	 * First check the validity of the extents described by the
	 * EFI.  If any are bad, then assume that all are bad and
	 * just toss the EFI.
	 */
	for (i = 0; i < efip->efi_format.efi_nextents; i++) {
		if (!xfs_efi_validate_ext(mp,
					&efip->efi_format.efi_extents[i])) {
			XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, mp,
					&efip->efi_format,
					sizeof(efip->efi_format));
			return -EFSCORRUPTED;
		}
	}

	resv = xlog_recover_resv(&M_RES(mp)->tr_itruncate);
	error = xfs_trans_alloc(mp, &resv, 0, 0, 0, &tp);
	if (error)
		return error;
	efdp = xfs_trans_get_efd(tp, efip, efip->efi_format.efi_nextents);

	for (i = 0; i < efip->efi_format.efi_nextents; i++) {
		struct xfs_extent_free_item	fake = {
			.xefi_owner		= XFS_RMAP_OWN_UNKNOWN,
			.xefi_agresv		= XFS_AG_RESV_NONE,
		};
		struct xfs_extent		*extp;

		extp = &efip->efi_format.efi_extents[i];

		fake.xefi_startblock = extp->ext_start;
		fake.xefi_blockcount = extp->ext_len;

		xfs_extent_free_get_group(mp, &fake);
		error = xfs_trans_free_extent(tp, efdp, &fake);
		xfs_extent_free_put_group(&fake);
		if (error == -EFSCORRUPTED)
			XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, mp,
					extp, sizeof(*extp));
		if (error)
			goto abort_error;

	}

	return xfs_defer_ops_capture_and_commit(tp, capture_list);

abort_error:
	xfs_trans_cancel(tp);
	return error;
}

STATIC bool
xfs_efi_item_match(
	struct xfs_log_item	*lip,
	uint64_t		intent_id)
{
	return EFI_ITEM(lip)->efi_format.efi_id == intent_id;
}

/* Relog an intent item to push the log tail forward. */
static struct xfs_log_item *
xfs_efi_item_relog(
	struct xfs_log_item		*intent,
	struct xfs_trans		*tp)
{
	struct xfs_efd_log_item		*efdp;
	struct xfs_efi_log_item		*efip;
	struct xfs_extent		*extp;
	unsigned int			count;

	count = EFI_ITEM(intent)->efi_format.efi_nextents;
	extp = EFI_ITEM(intent)->efi_format.efi_extents;

	tp->t_flags |= XFS_TRANS_DIRTY;
	efdp = xfs_trans_get_efd(tp, EFI_ITEM(intent), count);
	efdp->efd_next_extent = count;
	memcpy(efdp->efd_format.efd_extents, extp, count * sizeof(*extp));
	set_bit(XFS_LI_DIRTY, &efdp->efd_item.li_flags);

	efip = xfs_efi_init(tp->t_mountp, count);
	memcpy(efip->efi_format.efi_extents, extp, count * sizeof(*extp));
	atomic_set(&efip->efi_next_extent, count);
	xfs_trans_add_item(tp, &efip->efi_item);
	set_bit(XFS_LI_DIRTY, &efip->efi_item.li_flags);
	return &efip->efi_item;
}

static const struct xfs_item_ops xfs_efi_item_ops = {
	.flags		= XFS_ITEM_INTENT,
	.iop_size	= xfs_efi_item_size,
	.iop_format	= xfs_efi_item_format,
	.iop_unpin	= xfs_efi_item_unpin,
	.iop_release	= xfs_efi_item_release,
	.iop_recover	= xfs_efi_item_recover,
	.iop_match	= xfs_efi_item_match,
	.iop_relog	= xfs_efi_item_relog,
};

/*
 * This routine is called to create an in-core extent free intent
 * item from the efi format structure which was logged on disk.
 * It allocates an in-core efi, copies the extents from the format
 * structure into it, and adds the efi to the AIL with the given
 * LSN.
 */
STATIC int
xlog_recover_efi_commit_pass2(
	struct xlog			*log,
	struct list_head		*buffer_list,
	struct xlog_recover_item	*item,
	xfs_lsn_t			lsn)
{
	struct xfs_mount		*mp = log->l_mp;
	struct xfs_efi_log_item		*efip;
	struct xfs_efi_log_format	*efi_formatp;
	int				error;

	efi_formatp = item->ri_buf[0].i_addr;

	if (item->ri_buf[0].i_len < xfs_efi_log_format_sizeof(0)) {
		XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, mp,
				item->ri_buf[0].i_addr, item->ri_buf[0].i_len);
		return -EFSCORRUPTED;
	}

	efip = xfs_efi_init(mp, efi_formatp->efi_nextents);
	error = xfs_efi_copy_format(&item->ri_buf[0], &efip->efi_format);
	if (error) {
		xfs_efi_item_free(efip);
		return error;
	}
	atomic_set(&efip->efi_next_extent, efi_formatp->efi_nextents);
	/*
	 * Insert the intent into the AIL directly and drop one reference so
	 * that finishing or canceling the work will drop the other.
	 */
	xfs_trans_ail_insert(log->l_ailp, &efip->efi_item, lsn);
	xfs_efi_release(efip);
	return 0;
}

const struct xlog_recover_item_ops xlog_efi_item_ops = {
	.item_type		= XFS_LI_EFI,
	.commit_pass2		= xlog_recover_efi_commit_pass2,
};

/*
 * This routine is called when an EFD format structure is found in a committed
 * transaction in the log. Its purpose is to cancel the corresponding EFI if it
 * was still in the log. To do this it searches the AIL for the EFI with an id
 * equal to that in the EFD format structure. If we find it we drop the EFD
 * reference, which removes the EFI from the AIL and frees it.
 */
STATIC int
xlog_recover_efd_commit_pass2(
	struct xlog			*log,
	struct list_head		*buffer_list,
	struct xlog_recover_item	*item,
	xfs_lsn_t			lsn)
{
	struct xfs_efd_log_format	*efd_formatp;
	int				buflen = item->ri_buf[0].i_len;

	efd_formatp = item->ri_buf[0].i_addr;

	if (buflen < sizeof(struct xfs_efd_log_format)) {
		XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, log->l_mp,
				efd_formatp, buflen);
		return -EFSCORRUPTED;
	}

	if (item->ri_buf[0].i_len != xfs_efd_log_format32_sizeof(
						efd_formatp->efd_nextents) &&
	    item->ri_buf[0].i_len != xfs_efd_log_format64_sizeof(
						efd_formatp->efd_nextents)) {
		XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, log->l_mp,
				efd_formatp, buflen);
		return -EFSCORRUPTED;
	}

	xlog_recover_release_intent(log, XFS_LI_EFI, efd_formatp->efd_efi_id);
	return 0;
}

const struct xlog_recover_item_ops xlog_efd_item_ops = {
	.item_type		= XFS_LI_EFD,
	.commit_pass2		= xlog_recover_efd_commit_pass2,
};
