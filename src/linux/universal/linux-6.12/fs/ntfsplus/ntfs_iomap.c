// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * iomap callack functions
 *
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#include <linux/writeback.h>
#include <linux/mpage.h>
#include <linux/uio.h>

#include "aops.h"
#include "attrib.h"
#include "mft.h"
#include "ntfs.h"
#include "misc.h"
#include "ntfs_iomap.h"

static void ntfs_iomap_put_folio(struct inode *inode, loff_t pos,
		unsigned int len, struct folio *folio)
{
	struct ntfs_inode *ni = NTFS_I(inode);
	unsigned long sector_size = 1UL << inode->i_blkbits;
	loff_t start_down, end_up, init;

	if (!NInoNonResident(ni))
		goto out;

	start_down = round_down(pos, sector_size);
	end_up = (pos + len - 1) | (sector_size - 1);
	init = ni->initialized_size;

	if (init >= start_down && init <= end_up) {
		if (init < pos) {
			loff_t offset = offset_in_folio(folio, pos + len);

			if (offset == 0)
				offset = folio_size(folio);
			folio_zero_segments(folio,
					    offset_in_folio(folio, init),
					    offset_in_folio(folio, pos),
					    offset,
					    folio_size(folio));

		} else  {
			loff_t offset = max_t(loff_t, pos + len, init);

			offset = offset_in_folio(folio, offset);
			if (offset == 0)
				offset = folio_size(folio);
			folio_zero_segment(folio,
					   offset,
					   folio_size(folio));
		}
	} else if (init <= pos) {
		loff_t offset = 0, offset2 = offset_in_folio(folio, pos + len);

		if ((init >> folio_shift(folio)) == (pos >> folio_shift(folio)))
			offset = offset_in_folio(folio, init);
		if (offset2 == 0)
			offset2 = folio_size(folio);
		folio_zero_segments(folio,
				    offset,
				    offset_in_folio(folio, pos),
				    offset2,
				    folio_size(folio));
	}

out:
	folio_unlock(folio);
	folio_put(folio);
}

const struct iomap_folio_ops ntfs_iomap_folio_ops = {
	.put_folio = ntfs_iomap_put_folio,
};

static int ntfs_read_iomap_begin(struct inode *inode, loff_t offset, loff_t length,
		unsigned int flags, struct iomap *iomap, struct iomap *srcmap)
{
	struct ntfs_inode *base_ni, *ni = NTFS_I(inode);
	struct ntfs_attr_search_ctx *ctx;
	loff_t i_size;
	u32 attr_len;
	int err = 0;
	char *kattr;
	struct page *ipage;

	if (NInoNonResident(ni)) {
		s64 vcn;
		s64 lcn;
		struct runlist_element *rl;
		struct ntfs_volume *vol = ni->vol;
		loff_t vcn_ofs;
		loff_t rl_length;

		vcn = offset >> vol->cluster_size_bits;
		vcn_ofs = offset & vol->cluster_size_mask;

		down_write(&ni->runlist.lock);
		rl = ntfs_attr_vcn_to_rl(ni, vcn, &lcn);
		if (IS_ERR(rl)) {
			up_write(&ni->runlist.lock);
			return PTR_ERR(rl);
		}

		if (flags & IOMAP_REPORT) {
			if (lcn < LCN_HOLE) {
				up_write(&ni->runlist.lock);
				return -ENOENT;
			}
		} else if (lcn < LCN_ENOENT) {
			up_write(&ni->runlist.lock);
			return -EINVAL;
		}

		iomap->bdev = inode->i_sb->s_bdev;
		iomap->offset = offset;

		if (lcn <= LCN_DELALLOC) {
			if (lcn == LCN_DELALLOC)
				iomap->type = IOMAP_DELALLOC;
			else
				iomap->type = IOMAP_HOLE;
			iomap->addr = IOMAP_NULL_ADDR;
		} else {
			if (!(flags & IOMAP_ZERO) && offset >= ni->initialized_size)
				iomap->type = IOMAP_UNWRITTEN;
			else
				iomap->type = IOMAP_MAPPED;
			iomap->addr = (lcn << vol->cluster_size_bits) + vcn_ofs;
		}

		rl_length = (rl->length - (vcn - rl->vcn)) << ni->vol->cluster_size_bits;

		if (rl_length == 0 && rl->lcn > LCN_DELALLOC) {
			ntfs_error(inode->i_sb,
				   "runlist(vcn : %lld, length : %lld, lcn : %lld) is corrupted\n",
				   rl->vcn, rl->length, rl->lcn);
			up_write(&ni->runlist.lock);
			return -EIO;
		}

		if (rl_length && length > rl_length - vcn_ofs)
			iomap->length = rl_length - vcn_ofs;
		else
			iomap->length = length;
		up_write(&ni->runlist.lock);

		if (!(flags & IOMAP_ZERO) &&
		    iomap->type == IOMAP_MAPPED &&
		    iomap->offset < ni->initialized_size &&
		    iomap->offset + iomap->length > ni->initialized_size) {
			iomap->length = round_up(ni->initialized_size, 1 << inode->i_blkbits) -
				iomap->offset;
		}
		iomap->flags |= IOMAP_F_MERGED;
		return 0;
	}

	if (NInoAttr(ni))
		base_ni = ni->ext.base_ntfs_ino;
	else
		base_ni = ni;

	ctx = ntfs_attr_get_search_ctx(base_ni, NULL);
	if (!ctx) {
		err = -ENOMEM;
		goto out;
	}

	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err))
		goto out;

	attr_len = le32_to_cpu(ctx->attr->data.resident.value_length);
	if (unlikely(attr_len > ni->initialized_size))
		attr_len = ni->initialized_size;
	i_size = i_size_read(inode);

	if (unlikely(attr_len > i_size)) {
		/* Race with shrinking truncate. */
		attr_len = i_size;
	}

	if (offset >= attr_len) {
		if (flags & IOMAP_REPORT)
			err = -ENOENT;
		else
			err = -EFAULT;
		goto out;
	}

	kattr = (u8 *)ctx->attr + le16_to_cpu(ctx->attr->data.resident.value_offset);

	ipage = alloc_page(__GFP_NOWARN | __GFP_IO | __GFP_ZERO);
	if (!ipage) {
		err = -ENOMEM;
		goto out;
	}

	memcpy(page_address(ipage), kattr, attr_len);
	iomap->type = IOMAP_INLINE;
	iomap->inline_data = page_address(ipage);
	iomap->offset = 0;
	iomap->length = min_t(loff_t, attr_len, PAGE_SIZE);
	iomap->private = ipage;
	iomap->folio_ops = &ntfs_iomap_folio_ops;

out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	return err;
}

static int ntfs_read_iomap_end(struct inode *inode, loff_t pos, loff_t length,
		ssize_t written, unsigned int flags, struct iomap *iomap)
{
	if (iomap->type == IOMAP_INLINE) {
		struct page *ipage = iomap->private;

		put_page(ipage);
	}
	return written;
}

const struct iomap_ops ntfs_read_iomap_ops = {
	.iomap_begin = ntfs_read_iomap_begin,
	.iomap_end = ntfs_read_iomap_end,
};

static int ntfs_buffered_zeroed_clusters(struct inode *vi, s64 vcn)
{
	struct ntfs_inode *ni = NTFS_I(vi);
	struct ntfs_volume *vol = ni->vol;
	struct address_space *mapping = vi->i_mapping;
	struct folio *folio;
	pgoff_t idx, idx_end;
	u32 from, to;

	idx = (vcn << vol->cluster_size_bits) >> PAGE_SHIFT;
	idx_end = ((vcn + 1) << vol->cluster_size_bits) >> PAGE_SHIFT;
	from = (vcn << vol->cluster_size_bits) & ~PAGE_MASK;
	if (idx == idx_end)
		idx_end++;

	to = min_t(u32, vol->cluster_size, PAGE_SIZE);
	for (; idx < idx_end; idx++, from = 0) {
		if (to != PAGE_SIZE) {
			folio = ntfs_read_mapping_folio(mapping, idx);
			if (IS_ERR(folio))
				return PTR_ERR(folio);
			folio_lock(folio);
		} else {
			folio = __filemap_get_folio(mapping, idx,
					FGP_WRITEBEGIN | FGP_NOFS, mapping_gfp_mask(mapping));
			if (IS_ERR(folio))
				return PTR_ERR(folio);
		}

		if (folio_test_uptodate(folio) ||
		    iomap_is_partially_uptodate(folio, from, to))
			goto next_folio;

		folio_zero_segment(folio, from, from + to);
		folio_mark_uptodate(folio);

next_folio:
		iomap_dirty_folio(mapping, folio);
		folio_unlock(folio);
		folio_put(folio);
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
	}

	return 0;
}

int ntfs_zeroed_clusters(struct inode *vi, s64 lcn, s64 num)
{
	struct ntfs_inode *ni = NTFS_I(vi);
	struct ntfs_volume *vol = ni->vol;
	u32 to;
	struct bio *bio = NULL;
	s64 err = 0, zero_len = num << vol->cluster_size_bits;
	s64 loc = lcn << vol->cluster_size_bits, curr = 0;

	while (zero_len > 0) {
setup_bio:
		if (!bio) {
			bio = bio_alloc(vol->sb->s_bdev,
					bio_max_segs(DIV_ROUND_UP(zero_len, PAGE_SIZE)),
					REQ_OP_WRITE | REQ_SYNC | REQ_IDLE, GFP_NOIO);
			if (!bio)
				return -ENOMEM;
			bio->bi_iter.bi_sector = (loc + curr) >> vol->sb->s_blocksize_bits;
		}

		to = min_t(u32, zero_len, PAGE_SIZE);
		if (!bio_add_page(bio, ZERO_PAGE(0), to, 0)) {
			err = submit_bio_wait(bio);
			bio_put(bio);
			bio = NULL;
			if (err)
				break;
			goto setup_bio;
		}
		zero_len -= to;
		curr += to;
	}

	if (bio) {
		err = submit_bio_wait(bio);
		bio_put(bio);
	}

	return err;
}

static int __ntfs_write_iomap_begin(struct inode *inode, loff_t offset,
				    loff_t length, unsigned int flags,
				    struct iomap *iomap, bool da, bool mapped)
{
	struct ntfs_inode *ni = NTFS_I(inode);
	struct ntfs_volume *vol = ni->vol;
	struct attr_record *a;
	struct ntfs_attr_search_ctx *ctx;
	u32 attr_len;
	int err = 0;
	char *kattr;
	struct page *ipage;

	if (NVolShutdown(vol))
		return -EIO;

	mutex_lock(&ni->mrec_lock);
	if (NInoNonResident(ni)) {
		s64 vcn;
		loff_t vcn_ofs;
		loff_t rl_length;
		s64 max_clu_count =
			round_up(length, vol->cluster_size) >> vol->cluster_size_bits;

		vcn = offset >> vol->cluster_size_bits;
		vcn_ofs = offset & vol->cluster_size_mask;

		if (da) {
			bool balloc = false;
			s64 start_lcn, lcn_count;
			bool update_mp;

			update_mp = (flags & IOMAP_DIRECT) || mapped ||
				NInoAttr(ni) || ni->mft_no < FILE_first_user;
			down_write(&ni->runlist.lock);
			err = ntfs_attr_map_cluster(ni, vcn, &start_lcn, &lcn_count,
					max_clu_count, &balloc, update_mp,
					!(flags & IOMAP_DIRECT) && !mapped);
			up_write(&ni->runlist.lock);
			mutex_unlock(&ni->mrec_lock);
			if (err) {
				ni->i_dealloc_clusters = 0;
				return err;
			}

			iomap->bdev = inode->i_sb->s_bdev;
			iomap->offset = offset;

			rl_length = lcn_count << ni->vol->cluster_size_bits;
			if (length > rl_length - vcn_ofs)
				iomap->length = rl_length - vcn_ofs;
			else
				iomap->length = length;

			if (start_lcn == LCN_HOLE)
				iomap->type = IOMAP_HOLE;
			else
				iomap->type = IOMAP_MAPPED;
			if (balloc == true)
				iomap->flags = IOMAP_F_NEW;

			iomap->addr = (start_lcn << vol->cluster_size_bits) + vcn_ofs;

			if (balloc == true) {
				if (flags & IOMAP_DIRECT || mapped == true) {
					loff_t end = offset + length;

					if (vcn_ofs || ((vol->cluster_size > iomap->length) &&
							end < ni->initialized_size))
						err = ntfs_zeroed_clusters(inode,
								start_lcn, 1);
					if (!err && lcn_count > 1 &&
					    (iomap->length & vol->cluster_size_mask &&
					     end < ni->initialized_size))
						err = ntfs_zeroed_clusters(inode,
								start_lcn + (lcn_count - 1), 1);
				} else {
					if (lcn_count > ni->i_dealloc_clusters)
						ni->i_dealloc_clusters = 0;
					else
						ni->i_dealloc_clusters -= lcn_count;
				}
				if (err < 0)
					return err;
			}

			if (mapped && iomap->offset + iomap->length >
			    ni->initialized_size) {
				err = ntfs_attr_set_initialized_size(ni, iomap->offset +
								     iomap->length);
				if (err)
					return err;
			}
		} else {
			struct runlist_element *rl, *rlc;
			s64 lcn;
			bool is_retry = false;

			down_read(&ni->runlist.lock);
			rl = ni->runlist.rl;
			if (!rl) {
				up_read(&ni->runlist.lock);
				err = ntfs_map_runlist(ni, vcn);
				if (err) {
					mutex_unlock(&ni->mrec_lock);
					return -ENOENT;
				}
				down_read(&ni->runlist.lock);
				rl = ni->runlist.rl;
			}
			up_read(&ni->runlist.lock);

			down_write(&ni->runlist.lock);
remap_rl:
			/* Seek to element containing target vcn. */
			while (rl->length && rl[1].vcn <= vcn)
				rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, vcn);

			if (lcn <= LCN_RL_NOT_MAPPED && is_retry == false) {
				is_retry = true;
				if (!ntfs_map_runlist_nolock(ni, vcn, NULL)) {
					rl = ni->runlist.rl;
					goto remap_rl;
				}
			}

			max_clu_count = min(max_clu_count, rl->length - (vcn - rl->vcn));
			if (max_clu_count == 0) {
				ntfs_error(inode->i_sb,
					   "runlist(vcn : %lld, length : %lld) is corrupted\n",
					   rl->vcn, rl->length);
				up_write(&ni->runlist.lock);
				mutex_unlock(&ni->mrec_lock);
				return -EIO;
			}

			iomap->bdev = inode->i_sb->s_bdev;
			iomap->offset = offset;

			if (lcn <= LCN_DELALLOC) {
				if (lcn < LCN_DELALLOC) {
					max_clu_count =
						ntfs_available_clusters_count(vol, max_clu_count);
					if (max_clu_count < 0) {
						err = max_clu_count;
						up_write(&ni->runlist.lock);
						mutex_unlock(&ni->mrec_lock);
						return err;
					}
				}

				iomap->type = IOMAP_DELALLOC;
				iomap->addr = IOMAP_NULL_ADDR;

				if (lcn <= LCN_HOLE) {
					size_t new_rl_count;

					rlc = ntfs_malloc_nofs(sizeof(struct runlist_element) * 2);
					if (!rlc) {
						up_write(&ni->runlist.lock);
						mutex_unlock(&ni->mrec_lock);
						return -ENOMEM;
					}

					rlc->vcn = vcn;
					rlc->lcn = LCN_DELALLOC;
					rlc->length = max_clu_count;

					rlc[1].vcn = vcn + max_clu_count;
					rlc[1].lcn = LCN_RL_NOT_MAPPED;
					rlc[1].length = 0;

					rl = ntfs_runlists_merge(&ni->runlist, rlc, 0,
							&new_rl_count);
					if (IS_ERR(rl)) {
						ntfs_error(vol->sb, "Failed to merge runlists");
						up_write(&ni->runlist.lock);
						mutex_unlock(&ni->mrec_lock);
						ntfs_free(rlc);
						return PTR_ERR(rl);
					}

					ni->runlist.rl = rl;
					ni->runlist.count = new_rl_count;
					ni->i_dealloc_clusters += max_clu_count;
				}
				up_write(&ni->runlist.lock);
				mutex_unlock(&ni->mrec_lock);

				if (lcn < LCN_DELALLOC)
					ntfs_hold_dirty_clusters(vol, max_clu_count);

				rl_length = max_clu_count << ni->vol->cluster_size_bits;
				if (length > rl_length - vcn_ofs)
					iomap->length = rl_length - vcn_ofs;
				else
					iomap->length = length;

				iomap->flags = IOMAP_F_NEW;
				if (lcn <= LCN_HOLE) {
					loff_t end = offset + length;

					if (vcn_ofs || ((vol->cluster_size > iomap->length) &&
							end < ni->initialized_size))
						err = ntfs_buffered_zeroed_clusters(inode, vcn);
					if (!err && max_clu_count > 1 &&
					    (iomap->length & vol->cluster_size_mask &&
					     end < ni->initialized_size))
						err = ntfs_buffered_zeroed_clusters(inode,
								vcn + (max_clu_count - 1));
					if (err) {
						ntfs_release_dirty_clusters(vol, max_clu_count);
						return err;
					}
				}
			} else {
				up_write(&ni->runlist.lock);
				mutex_unlock(&ni->mrec_lock);

				iomap->type = IOMAP_MAPPED;
				iomap->addr = (lcn << vol->cluster_size_bits) + vcn_ofs;

				rl_length = max_clu_count << ni->vol->cluster_size_bits;
				if (length > rl_length - vcn_ofs)
					iomap->length = rl_length - vcn_ofs;
				else
					iomap->length = length;
			}
		}

		return 0;
	}

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx) {
		err = -ENOMEM;
		goto out;
	}

	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err) {
		if (err == -ENOENT)
			err = -EIO;
		goto out;
	}

	a = ctx->attr;
	/* The total length of the attribute value. */
	attr_len = le32_to_cpu(a->data.resident.value_length);
	kattr = (u8 *)a + le16_to_cpu(a->data.resident.value_offset);

	ipage = alloc_page(__GFP_NOWARN | __GFP_IO | __GFP_ZERO);
	if (!ipage) {
		err = -ENOMEM;
		goto out;
	}
	memcpy(page_address(ipage), kattr, attr_len);

	iomap->type = IOMAP_INLINE;
	iomap->inline_data = page_address(ipage);
	iomap->offset = 0;
	/* iomap requires there is only one INLINE_DATA extent */
	iomap->length = attr_len;
	iomap->private = ipage;
	iomap->folio_ops = &ntfs_iomap_folio_ops;

out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	mutex_unlock(&ni->mrec_lock);

	return err;
}

static int ntfs_write_iomap_begin(struct inode *inode, loff_t offset,
				  loff_t length, unsigned int flags,
				  struct iomap *iomap, struct iomap *srcmap)
{
	return __ntfs_write_iomap_begin(inode, offset, length, flags, iomap,
			false, false);
}

static int ntfs_write_iomap_end(struct inode *inode, loff_t pos, loff_t length,
		ssize_t written, unsigned int flags, struct iomap *iomap)
{
	if (iomap->type == IOMAP_INLINE) {
		struct page *ipage = iomap->private;
		struct ntfs_inode *ni = NTFS_I(inode);
		struct ntfs_attr_search_ctx *ctx;
		u32 attr_len;
		int err;
		char *kattr;

		mutex_lock(&ni->mrec_lock);
		ctx = ntfs_attr_get_search_ctx(ni, NULL);
		if (!ctx) {
			written = -ENOMEM;
			mutex_unlock(&ni->mrec_lock);
			goto out;
		}

		err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
				CASE_SENSITIVE, 0, NULL, 0, ctx);
		if (err) {
			if (err == -ENOENT)
				err = -EIO;
			written = err;
			goto err_out;
		}

		/* The total length of the attribute value. */
		attr_len = le32_to_cpu(ctx->attr->data.resident.value_length);
		if (pos >= attr_len || pos + written > attr_len)
			goto err_out;

		kattr = (u8 *)ctx->attr + le16_to_cpu(ctx->attr->data.resident.value_offset);
		memcpy(kattr + pos, iomap_inline_data(iomap, pos), written);
		mark_mft_record_dirty(ctx->ntfs_ino);
err_out:
		ntfs_attr_put_search_ctx(ctx);
		put_page(ipage);
		mutex_unlock(&ni->mrec_lock);
	}

out:
	return written;
}

const struct iomap_ops ntfs_write_iomap_ops = {
	.iomap_begin		= ntfs_write_iomap_begin,
	.iomap_end		= ntfs_write_iomap_end,
};

static int ntfs_page_mkwrite_iomap_begin(struct inode *inode, loff_t offset,
				  loff_t length, unsigned int flags,
				  struct iomap *iomap, struct iomap *srcmap)
{
	return __ntfs_write_iomap_begin(inode, offset, length, flags, iomap,
			true, true);
}

const struct iomap_ops ntfs_page_mkwrite_iomap_ops = {
	.iomap_begin		= ntfs_page_mkwrite_iomap_begin,
	.iomap_end		= ntfs_write_iomap_end,
};

static int ntfs_dio_iomap_begin(struct inode *inode, loff_t offset,
				  loff_t length, unsigned int flags,
				  struct iomap *iomap, struct iomap *srcmap)
{
	return __ntfs_write_iomap_begin(inode, offset, length, flags, iomap,
			true, false);
}

const struct iomap_ops ntfs_dio_iomap_ops = {
	.iomap_begin		= ntfs_dio_iomap_begin,
	.iomap_end		= ntfs_write_iomap_end,
};

static int ntfs_map_blocks(struct iomap_writepage_ctx *wpc,
		struct inode *inode, loff_t offset, unsigned int len)
{
	if (offset < wpc->iomap.offset ||
	    offset >= wpc->iomap.offset + wpc->iomap.length) {
		int error;
		error =   __ntfs_write_iomap_begin(inode, offset,
				NTFS_I(inode)->allocated_size - offset,
				IOMAP_WRITE, &wpc->iomap, true, false);
		if (error)
			return error;
	}
	return 0;
}

const struct iomap_writeback_ops ntfs_writeback_ops = {
	.map_blocks		= ntfs_map_blocks,
};
