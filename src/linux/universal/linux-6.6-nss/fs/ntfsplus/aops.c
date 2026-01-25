// SPDX-License-Identifier: GPL-2.0-or-later
/*
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
#include "debug.h"
#include "iomap.h"

static s64 lcn_from_index(struct ntfs_volume *vol, struct ntfs_inode *ni,
		unsigned long index)
{
	s64 vcn;
	s64 lcn;

	vcn = ntfs_pidx_to_cluster(vol, index);

	down_read(&ni->runlist.lock);
	lcn = ntfs_attr_vcn_to_lcn_nolock(ni, vcn, false);
	up_read(&ni->runlist.lock);

	return lcn;
}

/*
 * ntfs_read_folio - Read data for a folio from the device
 * @file:	open file to which the folio @folio belongs or NULL
 * @folio:	page cache folio to fill with data
 *
 * This function handles reading data into the page cache. It first checks
 * for specific ntfs attribute type like encryption and compression.
 *
 * - If the attribute is encrypted, access is denied (-EACCES) because
 *   decryption is not supported in this path.
 * - If the attribute is non-resident and compressed, the read operation is
 *   delegated to ntfs_read_compressed_block().
 * - For normal resident or non-resident attribute, it utilizes the generic
 *   iomap infrastructure via iomap_bio_read_folio() to perform the I/O.
 *
 * Return: 0 on success, or -errno on error.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
static int ntfs_read_folio(struct file *file, struct folio *folio)
#else
static int ntfs_readpage(struct file *file, struct page *page)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	struct ntfs_inode *ni = NTFS_I(folio->mapping->host);

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
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	struct page *page = &folio->page;
#endif
	loff_t i_size;
	struct inode *vi;
	struct ntfs_inode *ni;

	BUG_ON(!PageLocked(page));
	vi = page->mapping->host;
	i_size = i_size_read(vi);
	/* Is the page fully outside i_size? (truncate in progress) */
	if (unlikely(page->index >= (i_size + PAGE_SIZE - 1) >>
			PAGE_SHIFT)) {
		zero_user(page, 0, PAGE_SIZE);
		ntfs_debug("Read outside i_size - truncated?");
		SetPageUptodate(page);
		unlock_page(page);
		return 0;
	}
	/*
	 * This can potentially happen because we clear PageUptodate() during
	 * ntfs_writepage() of MstProtected() attributes.
	 */
	if (PageUptodate(page)) {
		unlock_page(page);
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
			BUG_ON(ni->type != AT_DATA);
			unlock_page(page);
			return -EACCES;
		}
		/* Compressed data streams are handled in compress.c. */
		if (NInoNonResident(ni) && NInoCompressed(ni)) {
			BUG_ON(ni->type != AT_DATA);
			BUG_ON(ni->name_len);
			return ntfs_read_compressed_block(page);
		}
	}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 19, 0)
	iomap_bio_read_folio(folio, &ntfs_read_iomap_ops);
	return 0;
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	return iomap_read_folio(folio, &ntfs_read_iomap_ops);
#else
	return iomap_readpage(page, &ntfs_read_iomap_ops);
#endif
#endif
}

/*
 * ntfs_bio_end_io - bio completion callback for MFT record writes
 *
 * Decrements the folio reference count that was incremented before
 * submit_bio(). This prevents a race condition where umount could
 * evict the inode and release the folio while I/O is still in flight,
 * potentially causing data corruption or use-after-free.
 */
void ntfs_bio_end_io(struct bio *bio)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	if (bio->bi_private)
		folio_put((struct folio *)bio->bi_private);
#else
	if (bio->bi_private)
		put_page((struct page *)bio->bi_private);
#endif
	bio_put(bio);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
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
	s64 vcn = ntfs_pidx_to_cluster(vol, folio->index);
	s64 end_vcn = ntfs_bytes_to_cluster(vol, ni->allocated_size);
	unsigned int folio_sz;
	struct runlist_element *rl;

	ntfs_debug("Entering for inode 0x%lx, attribute type 0x%x, folio index 0x%lx.",
			vi->i_ino, ni->type, folio->index);

	lcn = lcn_from_index(vol, ni, folio->index);
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
		vcn = ntfs_mft_no_to_cluster(vol, mft_no);
		/* Check whether to write this mft record. */
		tni = NULL;
		if (ntfs_may_write_mft_record(vol, mft_no,
					(struct mft_record *)(kaddr + mft_ofs), &tni)) {
			unsigned int mft_record_off = 0;
			s64 vcn_off = vcn;

			/*
			 * Skip $MFT extent mft records and let them being written
			 * by writeback to avioid deadlocks. the $MFT runlist
			 * lock must be taken before $MFT extent mrec_lock is taken.
			 */
			if (tni && tni->nr_extents < 0 &&
				tni->ext.base_ntfs_ino == NTFS_I(vol->mft_ino)) {
				mutex_unlock(&tni->mrec_lock);
				atomic_dec(&tni->count);
				iput(vol->mft_ino);
				continue;
			}

			/*
			 * The record should be written.  If a locked ntfs
			 * inode was returned, add it to the array of locked
			 * ntfs inodes.
			 */
			if (tni)
				locked_nis[nr_locked_nis++] = tni;

			if (bio && (mft_ofs != prev_mft_ofs + vol->mft_record_size)) {
flush_bio:
				bio->bi_end_io = ntfs_bio_end_io;
				submit_bio(bio);
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
					bio->bi_end_io = ntfs_bio_end_io;
					submit_bio(bio);
					bio = NULL;
				}
			}

			if (!bio) {
				unsigned int off;

				off = ((mft_no << vol->mft_record_size_bits) +
				       mft_record_off) & vol->cluster_size_mask;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
				bio = bio_alloc(vol->sb->s_bdev, 1, REQ_OP_WRITE,
						GFP_NOIO);
#else
				bio = bio_alloc(GFP_NOIO, 1);
				if (!bio)
					return NULL;
				bio_set_dev(bio, vol->sb->s_bdev);
				bio->bi_opf = REQ_OP_WRITE;
#endif
				bio->bi_iter.bi_sector =
					ntfs_bytes_to_sector(vol,
							ntfs_cluster_to_bytes(vol, lcn) + off);
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
		bio->bi_end_io = ntfs_bio_end_io;
		submit_bio(bio);
	}
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
#else
static int ntfs_write_mft_block(struct ntfs_inode *ni, struct page *page,
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
	s64 vcn = (s64)page->index << PAGE_SHIFT >> vol->cluster_size_bits;
	s64 end_vcn = ni->allocated_size >> vol->cluster_size_bits;
	unsigned int page_sz;
	struct runlist_element *rl;

	ntfs_debug("Entering for inode 0x%lx, attribute type 0x%x, page index 0x%lx.",
			vi->i_ino, ni->type, page->index);
	BUG_ON(!NInoNonResident(ni));
	BUG_ON(!NInoMstProtected(ni));

	/*
	 * NOTE: ntfs_write_mft_block() would be called for $MFTMirr if a page
	 * in its page cache were to be marked dirty.  However this should
	 * never happen with the current driver and considering we do not
	 * handle this case here we do want to BUG(), at least for now.
	 */

	BUG_ON(!((S_ISREG(vi->i_mode) && !vi->i_ino) || S_ISDIR(vi->i_mode) ||
		(NInoAttr(ni) && ni->type == AT_INDEX_ALLOCATION)));

	lcn = lcn_from_index(vol, ni, page->index);
	if (lcn <= LCN_HOLE) {
		set_page_writeback(page);
		unlock_page(page);
		end_page_writeback(page);
		return -EIO;
	}

	/* Map the page so we can access its contents. */
	kaddr = kmap(page);
	/* Clear the page uptodate flag whilst the mst fixups are applied. */
	BUG_ON(!PageUptodate(page));
	ClearPageUptodate(page);

	for (mft_ofs = 0; mft_ofs < PAGE_SIZE && vcn < end_vcn;
	     mft_ofs += vol->mft_record_size) {
		/* Get the mft record number. */
		mft_no = (((s64)page->index << PAGE_SHIFT) + mft_ofs) >>
			vol->mft_record_size_bits;
		vcn = mft_no << vol->mft_record_size_bits >> vol->cluster_size_bits;
		/* Check whether to write this mft record. */
		tni = NULL;
		if (ntfs_may_write_mft_record(vol, mft_no,
					(struct mft_record *)(kaddr + mft_ofs), &tni)) {
			unsigned int mft_record_off = 0;
			s64 vcn_off = vcn;

			/*
			 * Skip $MFT extent mft records and let them being written
			 * by writeback to avioid deadlocks. the $MFT runlist
			 * lock must be taken before $MFT extent mrec_lock is taken.
			 */
			if (tni && tni->nr_extents < 0 &&
				tni->ext.base_ntfs_ino == NTFS_I(vol->mft_ino)) {
				mutex_unlock(&tni->mrec_lock);
				atomic_dec(&tni->count);
				iput(vol->mft_ino);
				continue;
			}

			/*
			 * The record should be written.  If a locked ntfs
			 * inode was returned, add it to the array of locked
			 * ntfs inodes.
			 */
			if (tni)
				locked_nis[nr_locked_nis++] = tni;

			if (bio && (mft_ofs != prev_mft_ofs + vol->mft_record_size)) {
flush_bio:
				bio->bi_end_io = ntfs_bio_end_io;
				submit_bio(bio);
				bio = NULL;
			}

			if (vol->cluster_size < PAGE_SIZE) {
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
					bio->bi_end_io = ntfs_bio_end_io;
					submit_bio(bio);
					bio = NULL;
				}
			}

			if (!bio) {
				unsigned int off;

				off = ((mft_no << vol->mft_record_size_bits) +
				       mft_record_off) & vol->cluster_size_mask;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
				bio = bio_alloc(vol->sb->s_bdev, 1, REQ_OP_WRITE,
						GFP_NOIO);
#else
				bio = bio_alloc(GFP_NOIO, 1);
				if (!bio)
					return NULL;
				bio_set_dev(bio, vol->sb->s_bdev);
				bio->bi_opf = REQ_OP_WRITE;
#endif
				bio->bi_iter.bi_sector =
					ntfs_bytes_to_sector(vol,
							ntfs_cluster_to_bytes(vol, lcn) + off);
			}

			if (vol->cluster_size == NTFS_BLOCK_SIZE &&
			    (mft_record_off ||
			     rl->length - (vcn_off - rl->vcn) == 1 ||
			     mft_ofs + NTFS_BLOCK_SIZE >= PAGE_SIZE))
				page_sz = NTFS_BLOCK_SIZE;
			else
				page_sz = vol->mft_record_size;
			if (!bio_add_page(bio, page, page_sz,
					  mft_ofs + mft_record_off)) {
				err = -EIO;
				bio_put(bio);
				goto unm_done;
			}
			mft_record_off += page_sz;

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
		bio->bi_end_io = ntfs_bio_end_io;
		submit_bio(bio);
	}
unm_done:
	SetPageUptodate(page);
	kunmap(page);

	set_page_writeback(page);
	unlock_page(page);
	end_page_writeback(page);

	/* Unlock any locked inodes. */
	while (nr_locked_nis-- > 0) {
		struct ntfs_inode *base_tni;

		tni = locked_nis[nr_locked_nis];
		mutex_unlock(&tni->mrec_lock);

		/* Get the base inode. */
		mutex_lock(&tni->extent_lock);
		if (tni->nr_extents >= 0)
			base_tni = tni;
		else {
			base_tni = tni->ext.base_ntfs_ino;
			BUG_ON(!base_tni);
		}
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
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
/*
 * ntfs_writepage - write a @page to the backing store
 * @page:	page cache page to write out
 * @wbc:	writeback control structure
 *
 * This is called from the VM when it wants to have a dirty ntfs page cache
 * page cleaned.  The VM has already locked the page and marked it clean.
 *
 * For non-resident attributes, ntfs_writepage() writes the @page by calling
 * the ntfs version of the generic block_write_full_page() function,
 * ntfs_write_block(), which in turn if necessary creates and writes the
 * buffers associated with the page asynchronously.
 *
 * For resident attributes, OTOH, ntfs_writepage() writes the @page by copying
 * the data to the mft record (which at this stage is most likely in memory).
 * The mft record is then marked dirty and written out asynchronously via the
 * vfs inode dirty code path for the inode the mft record belongs to or via the
 * vm page dirty code path for the page the mft record is in.
 *
 * Based on ntfs_read_folio() and fs/buffer.c::block_write_full_page().
 */
static int ntfs_writepage(struct page *page, struct writeback_control *wbc)
{
	loff_t i_size;
	struct inode *vi = page->mapping->host;
	struct ntfs_inode *ni = NTFS_I(vi);
	struct iomap_writepage_ctx wpc = { };

	BUG_ON(!PageLocked(page));

	if (!NInoNonResident(ni)) {
		unlock_page(page);
		return 0;
	}

	i_size = i_size_read(vi);
	/* Is the page fully outside i_size? (truncate in progress) */
	if (unlikely(page->index >= (i_size + PAGE_SIZE - 1) >>
			PAGE_SHIFT)) {
		/*
		 * The page may have dirty, unmapped buffers.  Make them
		 * freeable here, so the page does not leak.
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
		struct folio *folio = page_folio(page);

		iomap_invalidate_folio(folio, 0, PAGE_SIZE);
#else
		iomap_invalidatepage(page, 0, PAGE_SIZE);
#endif
		unlock_page(page);
		ntfs_debug("Write outside i_size - truncated?");
		return 0;
	}
	/*
	 * Only $DATA attributes can be encrypted and only unnamed $DATA
	 * attributes can be compressed.  Index root can have the flags set but
	 * this means to create compressed/encrypted files, not that the
	 * attribute is compressed/encrypted.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 */
	if (ni->type != AT_INDEX_ALLOCATION) {
		/* If file is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {
			unlock_page(page);
			BUG_ON(ni->type != AT_DATA);
			ntfs_debug("Denying write access to encrypted file.");
			return -EACCES;
		}
		/* Compressed data streams are handled in compress.c. */
		if (NInoNonResident(ni) && NInoCompressed(ni)) {
			BUG_ON(ni->type != AT_DATA);
			BUG_ON(ni->name_len);
			unlock_page(page);
			ntfs_error(vi->i_sb, "Writing to compressed files is not supported yet.  Sorry.");
			return -EOPNOTSUPP;
		}
	}

	return iomap_writepage(page, wbc, &wpc, &ntfs_writeback_ops);
}
#endif

/*
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
	unsigned char blocksize_bits;

	ntfs_debug("Entering for mft_no 0x%lx, logical block 0x%llx.",
			ni->mft_no, (unsigned long long)block);
	if (ni->type != AT_DATA || !NInoNonResident(ni) || NInoEncrypted(ni) ||
	    NInoMstProtected(ni)) {
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
	down_read(&ni->runlist.lock);
	lcn = ntfs_attr_vcn_to_lcn_nolock(ni, ntfs_bytes_to_cluster(vol, ofs),
			false);
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
		block = lcn = (ntfs_cluster_to_bytes(vol, lcn) + delta) >>
				blocksize_bits;
		/* If the block number was truncated return 0. */
		if (unlikely(block != lcn)) {
			ntfs_error(vol->sb,
				"Physical block 0x%llx is too large to be returned, returning 0.",
				(long long)lcn);
			return 0;
		}
	} else
		block = (ntfs_cluster_to_bytes(vol, lcn) + delta) >>
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 19, 0)
	iomap_bio_readahead(rac, &ntfs_read_iomap_ops);
#else
	iomap_readahead(rac, &ntfs_read_iomap_ops);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
static int ntfs_mft_writepage(struct folio *folio, struct writeback_control *wbc)
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int ntfs_mft_writepage(struct folio *folio, struct writeback_control *wbc,
		void *data)
#else
static int ntfs_mft_writepage(struct page *page, struct writeback_control *wbc,
		void *data)
#endif
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	struct page *page = &folio->page;
#endif
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
	struct address_space *mapping = folio->mapping;
#else
	struct address_space *mapping = data;
#endif
	struct inode *vi = mapping->host;
	struct ntfs_inode *ni = NTFS_I(vi);
	loff_t i_size;
	int ret;

	i_size = i_size_read(vi);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	/* We have to zero every time due to mmap-at-end-of-file. */
	if (folio->index >= (i_size >> PAGE_SHIFT)) {
		/* The page straddles i_size. */
		unsigned int ofs = i_size & ~PAGE_MASK;

		folio_zero_segment(folio, ofs, PAGE_SIZE);
	}

	ret = ntfs_write_mft_block(ni, folio, wbc);
#else
	/* We have to zero every time due to mmap-at-end-of-file. */
	if (page->index >= (i_size >> PAGE_SHIFT)) {
		/* The page straddles i_size. */
		unsigned int ofs = i_size & ~PAGE_MASK;

		zero_user_segment(page, ofs, PAGE_SIZE);
	}

	ret = ntfs_write_mft_block(ni, page, wbc);
#endif

	mapping_set_error(mapping, ret);
	return ret;
}

static int ntfs_mft_writepages(struct address_space *mapping,
			       struct writeback_control *wbc)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
	struct folio *folio = NULL;
	int error;
#endif

	if (NVolShutdown(NTFS_I(mapping->host)->vol))
		return -EIO;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
	while ((folio = writeback_iter(mapping, wbc, folio, &error)))
		error = ntfs_mft_writepage(folio, wbc);
	return error;
#else
	return write_cache_pages(mapping, wbc,
				 ntfs_mft_writepage, mapping);
#endif
}

static int ntfs_writepages(struct address_space *mapping,
		struct writeback_control *wbc)
{
	struct inode *inode = mapping->host;
	struct ntfs_inode *ni = NTFS_I(inode);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0)
	struct iomap_writepage_ctx wpc = {
		.inode		= mapping->host,
		.wbc		= wbc,
		.ops		= &ntfs_writeback_ops,
	};
#else
	struct iomap_writepage_ctx wpc = { };
#endif

	if (NVolShutdown(ni->vol))
		return -EIO;

	if (!NInoNonResident(ni))
		return 0;

	if (NInoMstProtected(ni) && ni->mft_no == FILE_MFT) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
		struct folio *folio = NULL;
		int error;

		while ((folio = writeback_iter(mapping, wbc, folio, &error)))
			error = ntfs_mft_writepage(folio, wbc);
		return error;
#else
		return write_cache_pages(mapping, wbc,
				ntfs_mft_writepage, mapping);
#endif
	}

	/* If file is encrypted, deny access, just like NT4. */
	if (NInoEncrypted(ni)) {
		ntfs_debug("Denying write access to encrypted file.");
		return -EACCES;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0)
	return iomap_writepages(&wpc);
#else
	return iomap_writepages(mapping, wbc, &wpc, &ntfs_writeback_ops);
#endif
}

static int ntfs_swap_activate(struct swap_info_struct *sis,
		struct file *swap_file, sector_t *span)
{
	return iomap_swapfile_activate(sis, swap_file, span,
			&ntfs_read_iomap_ops);
}

const struct address_space_operations ntfs_aops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	.read_folio		= ntfs_read_folio,
#else
	.readpage		= ntfs_readpage,
#endif
	.readahead		= ntfs_readahead,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	.writepage		= ntfs_writepage,
#endif
	.writepages		= ntfs_writepages,
	.direct_IO		= noop_direct_IO,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	.dirty_folio		= iomap_dirty_folio,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	.dirty_folio		= filemap_dirty_folio,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
	.dirty_folio		= iomap_dirty_folio,
#else
	.set_page_dirty		= __set_page_dirty_nobuffers,
#endif
#endif
#endif
	.bmap			= ntfs_bmap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	.migrate_folio		= filemap_migrate_folio,
#else
	.migratepage		= iomap_migrate_page,
#endif
	.is_partially_uptodate	= iomap_is_partially_uptodate,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
	.error_remove_folio	= generic_error_remove_folio,
#else
	.error_remove_page	= generic_error_remove_page,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	.release_folio		= iomap_release_folio,
	.invalidate_folio	= iomap_invalidate_folio,
#else
	.releasepage		= iomap_releasepage,
	.invalidatepage		= iomap_invalidatepage,
#endif
	.swap_activate          = ntfs_swap_activate,
};

const struct address_space_operations ntfs_mft_aops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	.read_folio		= ntfs_read_folio,
#else
	.readpage		= ntfs_readpage,
#endif
	.readahead		= ntfs_readahead,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	.writepage		= ntfs_writepage,
#endif
	.writepages		= ntfs_mft_writepages,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	.dirty_folio		= iomap_dirty_folio,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	.dirty_folio		= filemap_dirty_folio,
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
	.dirty_folio		= iomap_dirty_folio,
#else
	.set_page_dirty		= __set_page_dirty_nobuffers,
#endif
#endif
#endif
	.bmap			= ntfs_bmap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	.migrate_folio		= filemap_migrate_folio,
#else
	.migratepage		= iomap_migrate_page,
#endif
	.is_partially_uptodate	= iomap_is_partially_uptodate,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
	.error_remove_folio	= generic_error_remove_folio,
#else
	.error_remove_page	= generic_error_remove_page,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	.release_folio		= iomap_release_folio,
	.invalidate_folio	= iomap_invalidate_folio,
#else
	.releasepage		= iomap_releasepage,
	.invalidatepage		= iomap_invalidatepage,
#endif
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
void mark_ntfs_record_dirty(struct folio *folio)
{
	iomap_dirty_folio(folio->mapping, folio);
#else
void mark_ntfs_record_dirty(struct page *page)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
	filemap_dirty_folio(page->mapping, page_folio(page));
#else
	__set_page_dirty_nobuffers(page);
#endif
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
int ntfs_dev_read(struct super_block *sb, void *buf, loff_t start, loff_t size)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct folio *folio;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
		folio = read_mapping_folio(sb->s_bdev->bd_mapping, idx, NULL);
#else
		folio = read_mapping_folio(sb->s_bdev->bd_inode->i_mapping, idx, NULL);
#endif
		if (IS_ERR(folio)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(folio);
		}

		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy_from_folio(buf + buf_off, folio, from, to);
		buf_off += to;
		folio_put(folio);
	}

	return 0;
}

int ntfs_dev_write(struct super_block *sb, void *buf, loff_t start, loff_t size)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct folio *folio;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
		folio = read_mapping_folio(sb->s_bdev->bd_mapping, idx, NULL);
#else
		folio = read_mapping_folio(sb->s_bdev->bd_inode->i_mapping, idx, NULL);
#endif
		if (IS_ERR(folio)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(folio);
		}

		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy_to_folio(folio, from, buf + buf_off, to);
		buf_off += to;
		folio_mark_uptodate(folio);
		folio_mark_dirty(folio);
		folio_put(folio);
	}

	return 0;
}
#else
int ntfs_dev_read(struct super_block *sb, void *buf, loff_t start, loff_t size)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct page *page;
	char *kaddr;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
		page = read_mapping_page(sb->s_bdev->bd_inode->i_mapping, idx, NULL);
		if (IS_ERR(page)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(page);
		}

		kaddr = kmap_atomic(page);
		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy(buf + buf_off, kaddr + from, to);
		buf_off += to;
		kunmap_atomic(kaddr);
		put_page(page);
	}

	return 0;
}

int ntfs_dev_write(struct super_block *sb, void *buf, loff_t start, loff_t size)
{
	pgoff_t idx, idx_end;
	loff_t offset, end = start + size;
	u32 from, to, buf_off = 0;
	struct page *page;
	char *kaddr;

	idx = start >> PAGE_SHIFT;
	idx_end = end >> PAGE_SHIFT;
	from = start & ~PAGE_MASK;

	if (idx == idx_end)
		idx_end++;

	for (; idx < idx_end; idx++, from = 0) {
		page = read_mapping_page(sb->s_bdev->bd_inode->i_mapping, idx, NULL);
		if (IS_ERR(page)) {
			ntfs_error(sb, "Unable to read %ld page", idx);
			return PTR_ERR(page);
		}

		kaddr = kmap_atomic(page);
		offset = (loff_t)idx << PAGE_SHIFT;
		to = min_t(u32, end - offset, PAGE_SIZE);

		memcpy(kaddr + from, buf + buf_off, to);
		buf_off += to;
		kunmap_atomic(kaddr);
		SetPageUptodate(page);
		set_page_dirty(page);
		put_page(page);
	}

	return 0;
}
#endif
