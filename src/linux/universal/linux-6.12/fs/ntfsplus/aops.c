// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * NTFS kernel address space operations and page cache handling.
 *
 * Copyright (c) 2001-2014 Anton Altaparmakov and Tuxera Inc.
 * Copyright (c) 2002 Richard Russon
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

static s64 ntfs_convert_page_index_into_lcn(struct ntfs_volume *vol, struct ntfs_inode *ni,
		unsigned long page_index)
{
	sector_t iblock;
	s64 vcn;
	s64 lcn;
	unsigned char blocksize_bits = vol->sb->s_blocksize_bits;

	iblock = (s64)page_index << (PAGE_SHIFT - blocksize_bits);
	vcn = (s64)iblock << blocksize_bits >> vol->cluster_size_bits;

	down_read(&ni->runlist.lock);
	lcn = ntfs_attr_vcn_to_lcn_nolock(ni, vcn, false);
	up_read(&ni->runlist.lock);

	return lcn;
}

struct bio *ntfs_setup_bio(struct ntfs_volume *vol, blk_opf_t opf, s64 lcn,
		unsigned int pg_ofs)
{
	struct bio *bio;

	bio = bio_alloc(vol->sb->s_bdev, 1, opf, GFP_NOIO);
	if (!bio)
		return NULL;
	bio->bi_iter.bi_sector = ((lcn << vol->cluster_size_bits) + pg_ofs) >>
		vol->sb->s_blocksize_bits;

	return bio;
}

/**
 * ntfs_read_folio - fill a @folio of a @file with data from the device
 * @file:	open file to which the folio @folio belongs or NULL
 * @folio:	page cache folio to fill with data
 *
 * For non-resident attributes, ntfs_read_folio() fills the @folio of the open
 * file @file by calling the ntfs version of the generic block_read_full_folio()
 * function, which in turn creates and reads in the buffers associated with
 * the folio asynchronously.
 *
 * For resident attributes, OTOH, ntfs_read_folio() fills @folio by copying the
 * data from the mft record (which at this stage is most likely in memory) and
 * fills the remainder with zeroes. Thus, in this case, I/O is synchronous, as
 * even if the mft record is not cached at this point in time, we need to wait
 * for it to be read in before we can do the copy.
 *
 * Return 0 on success and -errno on error.
 */
static int ntfs_read_folio(struct file *file, struct folio *folio)
{
	loff_t i_size;
	struct inode *vi;
	struct ntfs_inode *ni;

	vi = folio->mapping->host;
	i_size = i_size_read(vi);
	/* Is the page fully outside i_size? (truncate in progress) */
	if (unlikely(folio->index >= (i_size + PAGE_SIZE - 1) >>
			PAGE_SHIFT)) {
		folio_zero_segment(folio, 0, PAGE_SIZE);
		ntfs_debug("Read outside i_size - truncated?");
		folio_mark_uptodate(folio);
		folio_unlock(folio);
		return 0;
	}
	/*
	 * This can potentially happen because we clear PageUptodate() during
	 * ntfs_writepage() of MstProtected() attributes.
	 */
	if (folio_test_uptodate(folio)) {
		folio_unlock(folio);
		return 0;
	}
	ni = NTFS_I(vi);

	/*
	 * Only $DATA attributes can be encrypted and only unnamed $DATA
	 * attributes can be compressed.  Index root can have the flags set but
	 * this means to create compressed/encrypted files, not that the
	 * attribute is compressed/encrypted.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 */
	if (ni->type != AT_INDEX_ALLOCATION) {
		/* If attribute is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {
			folio_unlock(folio);
			return -EACCES;
		}
		/* Compressed data streams are handled in compress.c. */
		if (NInoNonResident(ni) && NInoCompressed(ni))
			return ntfs_read_compressed_block(folio);
	}

	return iomap_read_folio(folio, &ntfs_read_iomap_ops);
}

static int ntfs_write_mft_block(struct ntfs_inode *ni, struct folio *folio,
		struct writeback_control *wbc)
{
	struct inode *vi = VFS_I(ni);
	struct ntfs_volume *vol = ni->vol;
	u8 *kaddr;
	struct ntfs_inode *locked_nis[PAGE_SIZE / NTFS_BLOCK_SIZE];
	int nr_locked_nis = 0, err = 0, mft_ofs, prev_mft_ofs;
	struct bio *bio = NULL;
	unsigned long mft_no;
	struct ntfs_inode *tni;
	s64 lcn;
	s64 vcn = (s64)folio->index << PAGE_SHIFT >> vol->cluster_size_bits;
	s64 end_vcn = ni->allocated_size >> vol->cluster_size_bits;
	unsigned int folio_sz;
	struct runlist_element *rl;

	ntfs_debug("Entering for inode 0x%lx, attribute type 0x%x, folio index 0x%lx.",
			vi->i_ino, ni->type, folio->index);

	lcn = ntfs_convert_page_index_into_lcn(vol, ni, folio->index);
	if (lcn <= LCN_HOLE) {
		folio_start_writeback(folio);
		folio_unlock(folio);
		folio_end_writeback(folio);
		return -EIO;
	}

	/* Map folio so we can access its contents. */
	kaddr = kmap_local_folio(folio, 0);
	/* Clear the page uptodate flag whilst the mst fixups are applied. */
	folio_clear_uptodate(folio);

	for (mft_ofs = 0; mft_ofs < PAGE_SIZE && vcn < end_vcn;
	     mft_ofs += vol->mft_record_size) {
		/* Get the mft record number. */
		mft_no = (((s64)folio->index << PAGE_SHIFT) + mft_ofs) >>
			vol->mft_record_size_bits;
		vcn = mft_no << vol->mft_record_size_bits >> vol->cluster_size_bits;
		/* Check whether to write this mft record. */
		tni = NULL;
		if (ntfs_may_write_mft_record(vol, mft_no,
					(struct mft_record *)(kaddr + mft_ofs), &tni)) {
			unsigned int mft_record_off = 0;
			s64 vcn_off = vcn;

			/*
			 * The record should be written.  If a locked ntfs
			 * inode was returned, add it to the array of locked
			 * ntfs inodes.
			 */
			if (tni)
				locked_nis[nr_locked_nis++] = tni;

			if (bio && (mft_ofs != prev_mft_ofs + vol->mft_record_size)) {
flush_bio:
				flush_dcache_folio(folio);
				submit_bio_wait(bio);
				bio_put(bio);
				bio = NULL;
			}

			if (vol->cluster_size < folio_size(folio)) {
				down_write(&ni->runlist.lock);
				rl = ntfs_attr_vcn_to_rl(ni, vcn_off, &lcn);
				up_write(&ni->runlist.lock);
				if (IS_ERR(rl) || lcn < 0) {
					err = -EIO;
					goto unm_done;
				}

				if (bio &&
				   (bio_end_sector(bio) >> (vol->cluster_size_bits - 9)) !=
				    lcn) {
					flush_dcache_folio(folio);
					submit_bio_wait(bio);
					bio_put(bio);
					bio = NULL;
				}
			}

			if (!bio) {
				unsigned int off;

				off = ((mft_no << vol->mft_record_size_bits) +
				       mft_record_off) & vol->cluster_size_mask;

				bio = ntfs_setup_bio(vol, REQ_OP_WRITE, lcn, off);
				if (!bio) {
					err = -ENOMEM;
					goto unm_done;
				}
			}

			if (vol->cluster_size == NTFS_BLOCK_SIZE &&
			    (mft_record_off ||
			     rl->length - (vcn_off - rl->vcn) == 1 ||
			     mft_ofs + NTFS_BLOCK_SIZE >= PAGE_SIZE))
				folio_sz = NTFS_BLOCK_SIZE;
			else
				folio_sz = vol->mft_record_size;
			if (!bio_add_folio(bio, folio, folio_sz,
					   mft_ofs + mft_record_off)) {
				err = -EIO;
				bio_put(bio);
				goto unm_done;
			}
			mft_record_off += folio_sz;

			if (mft_record_off != vol->mft_record_size) {
				vcn_off++;
				goto flush_bio;
			}
			prev_mft_ofs = mft_ofs;

			if (mft_no < vol->mftmirr_size)
				ntfs_sync_mft_mirror(vol, mft_no,
						(struct mft_record *)(kaddr + mft_ofs));
		}

	}

	if (bio) {
		flush_dcache_folio(folio);
		submit_bio_wait(bio);
		bio_put(bio);
	}
	flush_dcache_folio(folio);
unm_done:
	folio_mark_uptodate(folio);
	kunmap_local(kaddr);

	folio_start_writeback(folio);
	folio_unlock(folio);
	folio_end_writeback(folio);

	/* Unlock any locked inodes. */
	while (nr_locked_nis-- > 0) {
		struct ntfs_inode *base_tni;

		tni = locked_nis[nr_locked_nis];
		mutex_unlock(&tni->mrec_lock);

		/* Get the base inode. */
		mutex_lock(&tni->extent_lock);
		if (tni->nr_extents >= 0)
			base_tni = tni;
		else
			base_tni = tni->ext.base_ntfs_ino;
		mutex_unlock(&tni->extent_lock);
		ntfs_debug("Unlocking %s inode 0x%lx.",
				tni == base_tni ? "base" : "extent",
				tni->mft_no);
		atomic_dec(&tni->count);
		iput(VFS_I(base_tni));
	}

	if (unlikely(err && err != -ENOMEM))
		NVolSetErrors(vol);
	if (likely(!err))
		ntfs_debug("Done.");
	return err;
}

/**
 * ntfs_bmap - map logical file block to physical device block
 * @mapping:	address space mapping to which the block to be mapped belongs
 * @block:	logical block to map to its physical device block
 *
 * For regular, non-resident files (i.e. not compressed and not encrypted), map
 * the logical @block belonging to the file described by the address space
 * mapping @mapping to its physical device block.
 *
 * The size of the block is equal to the @s_blocksize field of the super block
 * of the mounted file system which is guaranteed to be smaller than or equal
 * to the cluster size thus the block is guaranteed to fit entirely inside the
 * cluster which means we do not need to care how many contiguous bytes are
 * available after the beginning of the block.
 *
 * Return the physical device block if the mapping succeeded or 0 if the block
 * is sparse or there was an error.
 *
 * Note: This is a problem if someone tries to run bmap() on $Boot system file
 * as that really is in block zero but there is nothing we can do.  bmap() is
 * just broken in that respect (just like it cannot distinguish sparse from
 * not available or error).
 */
static sector_t ntfs_bmap(struct address_space *mapping, sector_t block)
{
	s64 ofs, size;
	loff_t i_size;
	s64 lcn;
	unsigned long blocksize, flags;
	struct ntfs_inode *ni = NTFS_I(mapping->host);
	struct ntfs_volume *vol = ni->vol;
	unsigned int delta;
	unsigned char blocksize_bits, cluster_size_shift;

	ntfs_debug("Entering for mft_no 0x%lx, logical block 0x%llx.",
			ni->mft_no, (unsigned long long)block);
	if (ni->type != AT_DATA || !NInoNonResident(ni) || NInoEncrypted(ni)) {
		ntfs_error(vol->sb, "BMAP does not make sense for %s attributes, returning 0.",
				(ni->type != AT_DATA) ? "non-data" :
				(!NInoNonResident(ni) ? "resident" :
				"encrypted"));
		return 0;
	}
	/* None of these can happen. */
	blocksize = vol->sb->s_blocksize;
	blocksize_bits = vol->sb->s_blocksize_bits;
	ofs = (s64)block << blocksize_bits;
	read_lock_irqsave(&ni->size_lock, flags);
	size = ni->initialized_size;
	i_size = i_size_read(VFS_I(ni));
	read_unlock_irqrestore(&ni->size_lock, flags);
	/*
	 * If the offset is outside the initialized size or the block straddles
	 * the initialized size then pretend it is a hole unless the
	 * initialized size equals the file size.
	 */
	if (unlikely(ofs >= size || (ofs + blocksize > size && size < i_size)))
		goto hole;
	cluster_size_shift = vol->cluster_size_bits;
	down_read(&ni->runlist.lock);
	lcn = ntfs_attr_vcn_to_lcn_nolock(ni, ofs >> cluster_size_shift, false);
	up_read(&ni->runlist.lock);
	if (unlikely(lcn < LCN_HOLE)) {
		/*
		 * Step down to an integer to avoid gcc doing a long long
		 * comparision in the switch when we know @lcn is between
		 * LCN_HOLE and LCN_EIO (i.e. -1 to -5).
		 *
		 * Otherwise older gcc (at least on some architectures) will
		 * try to use __cmpdi2() which is of course not available in
		 * the kernel.
		 */
		switch ((int)lcn) {
		case LCN_ENOENT:
			/*
			 * If the offset is out of bounds then pretend it is a
			 * hole.
			 */
			goto hole;
		case LCN_ENOMEM:
			ntfs_error(vol->sb,
				"Not enough memory to complete mapping for inode 0x%lx. Returning 0.",
				ni->mft_no);
			break;
		default:
			ntfs_error(vol->sb,
				"Failed to complete mapping for inode 0x%lx.  Run chkdsk. Returning 0.",
				ni->mft_no);
			break;
		}
		return 0;
	}
	if (lcn < 0) {
		/* It is a hole. */
hole:
		ntfs_debug("Done (returning hole).");
		return 0;
	}
	/*
	 * The block is really allocated and fullfils all our criteria.
	 * Convert the cluster to units of block size and return the result.
	 */
	delta = ofs & vol->cluster_size_mask;
	if (unlikely(sizeof(block) < sizeof(lcn))) {
		block = lcn = ((lcn << cluster_size_shift) + delta) >>
				blocksize_bits;
		/* If the block number was truncated return 0. */
		if (unlikely(block != lcn)) {
			ntfs_error(vol->sb,
				"Physical block 0x%llx is too large to be returned, returning 0.",
				(long long)lcn);
			return 0;
		}
	} else
		block = ((lcn << cluster_size_shift) + delta) >>
				blocksize_bits;
	ntfs_debug("Done (returning block 0x%llx).", (unsigned long long)lcn);
	return block;
}

static void ntfs_readahead(struct readahead_control *rac)
{
	struct address_space *mapping = rac->mapping;
	struct inode *inode = mapping->host;
	struct ntfs_inode *ni = NTFS_I(inode);

	if (!NInoNonResident(ni) || NInoCompressed(ni)) {
		/* No readahead for resident and compressed. */
		return;
	}

	if (NInoMstProtected(ni) &&
	    (ni->mft_no == FILE_MFT || ni->mft_no == FILE_MFTMirr))
		return;

	iomap_readahead(rac, &ntfs_read_iomap_ops);
}

static int ntfs_mft_writepage(struct folio *folio, struct writeback_control *wbc)
{
	struct address_space *mapping = folio->mapping;
	struct inode *vi = mapping->host;
	struct ntfs_inode *ni = NTFS_I(vi);
	loff_t i_size;
	int ret;

	i_size = i_size_read(vi);

	/* We have to zero every time due to mmap-at-end-of-file. */
	if (folio->index >= (i_size >> PAGE_SHIFT)) {
		/* The page straddles i_size. */
		unsigned int ofs = i_size & ~PAGE_MASK;

		folio_zero_segment(folio, ofs, PAGE_SIZE);
	}

	ret = ntfs_write_mft_block(ni, folio, wbc);
	mapping_set_error(mapping, ret);
	return ret;
}

static int ntfs_writepages(struct address_space *mapping,
		struct writeback_control *wbc)
{
	struct inode *inode = mapping->host;
	struct ntfs_inode *ni = NTFS_I(inode);
	struct iomap_writepage_ctx wpc = { };

	if (NVolShutdown(ni->vol))
		return -EIO;

	if (!NInoNonResident(ni))
		return 0;

	if (NInoMstProtected(ni) && ni->mft_no == FILE_MFT) {
		struct folio *folio = NULL;
		int error;

		while ((folio = writeback_iter(mapping, wbc, folio, &error)))
			error = ntfs_mft_writepage(folio, wbc);
		return error;
	}

	/* If file is encrypted, deny access, just like NT4. */
	if (NInoEncrypted(ni)) {
		ntfs_debug("Denying write access to encrypted file.");
		return -EACCES;
	}

	return iomap_writepages(mapping, wbc, &wpc, &ntfs_writeback_ops);
}

static int ntfs_swap_activate(struct swap_info_struct *sis,
		struct file *swap_file, sector_t *span)
{
	return iomap_swapfile_activate(sis, swap_file, span,
			&ntfs_read_iomap_ops);
}

/**
 * ntfs_normal_aops - address space operations for normal inodes and attributes
 *
 * Note these are not used for compressed or mst protected inodes and
 * attributes.
 */
const struct address_space_operations ntfs_normal_aops = {
	.read_folio		= ntfs_read_folio,
	.readahead		= ntfs_readahead,
	.writepages		= ntfs_writepages,
	.direct_IO		= noop_direct_IO,
	.dirty_folio		= iomap_dirty_folio,
	.bmap			= ntfs_bmap,
	.migrate_folio		= filemap_migrate_folio,
	.is_partially_uptodate	= iomap_is_partially_uptodate,
	.error_remove_folio	= generic_error_remove_folio,
	.release_folio		= iomap_release_folio,
	.invalidate_folio	= iomap_invalidate_folio,
	.swap_activate          = ntfs_swap_activate,
};

/**
 * ntfs_compressed_aops - address space operations for compressed inodes
 */
const struct address_space_operations ntfs_compressed_aops = {
	.read_folio		= ntfs_read_folio,
	.direct_IO		= noop_direct_IO,
	.writepages		= ntfs_writepages,
	.dirty_folio		= iomap_dirty_folio,
	.migrate_folio		= filemap_migrate_folio,
	.is_partially_uptodate	= iomap_is_partially_uptodate,
	.error_remove_folio	= generic_error_remove_folio,
	.release_folio		= iomap_release_folio,
	.invalidate_folio	= iomap_invalidate_folio,
};

/**
 * ntfs_mst_aops - general address space operations for mst protecteed inodes
 *		   and attributes
 */
const struct address_space_operations ntfs_mst_aops = {
	.read_folio		= ntfs_read_folio,	/* Fill page with data. */
	.readahead		= ntfs_readahead,
	.writepages		= ntfs_writepages,	/* Write dirty page to disk. */
	.dirty_folio		= iomap_dirty_folio,
	.migrate_folio		= filemap_migrate_folio,
	.is_partially_uptodate	= iomap_is_partially_uptodate,
	.error_remove_folio	= generic_error_remove_folio,
	.release_folio		= iomap_release_folio,
	.invalidate_folio	= iomap_invalidate_folio,
};

void mark_ntfs_record_dirty(struct folio *folio)
{
	iomap_dirty_folio(folio->mapping, folio);
}

int ntfs_dev_read(struct super_block *sb, void *buf, loff_t start, loff_t size)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct folio *folio;
	char *kaddr;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
		folio = ntfs_read_mapping_folio(sb->s_bdev->bd_mapping, idx);
		if (IS_ERR(folio)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(folio);
		}

		kaddr = kmap_local_folio(folio, 0);
		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy(buf + buf_off, kaddr + from, to);
		buf_off += to;
		kunmap_local(kaddr);
		folio_put(folio);
	}

	return 0;
}

int ntfs_dev_write(struct super_block *sb, void *buf, loff_t start,
			loff_t size, bool wait)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct folio *folio;
	char *kaddr;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
		folio = ntfs_read_mapping_folio(sb->s_bdev->bd_mapping, idx);
		if (IS_ERR(folio)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(folio);
		}

		kaddr = kmap_local_folio(folio, 0);
		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy(kaddr + from, buf + buf_off, to);
		buf_off += to;
		kunmap_local(kaddr);
		folio_mark_uptodate(folio);
		folio_mark_dirty(folio);
		if (wait)
			folio_wait_stable(folio);
		folio_put(folio);
	}

	return 0;
}
