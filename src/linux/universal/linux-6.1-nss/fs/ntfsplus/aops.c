// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NTFS kernel address space operations and page cache handling.
 *
 * Copyright (c) 2001-2014 Anton Altaparmakov and Tuxera Inc.
 * Copyright (c) 2002 Richard Russon
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#include <linux/writeback.h>

#include "attrib.h"
#include "mft.h"
#include "ntfs.h"
#include "debug.h"
#include "iomap.h"

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
		/*
		 * EFS-encrypted files are not supported.
		 * (decryption/encryption is not implemented yet)
		 */
		if (NInoEncrypted(ni)) {
			folio_unlock(folio);
			return -EOPNOTSUPP;
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

	/*
	 * Resident files are not cached in the page cache,
	 * and readahead is not implemented for compressed files.
	 */
	if (!NInoNonResident(ni) || NInoCompressed(ni))
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 19, 0)
	iomap_bio_readahead(rac, &ntfs_read_iomap_ops);
#else
	iomap_readahead(rac, &ntfs_read_iomap_ops);
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

	/*
	 * EFS-encrypted files are not supported.
	 * (decryption/encryption is not implemented yet)
	 */
	if (NInoEncrypted(ni)) {
		ntfs_debug("Encrypted I/O not supported");
		return -EOPNOTSUPP;
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
