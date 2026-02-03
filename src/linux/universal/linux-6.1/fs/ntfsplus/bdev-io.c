// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NTFS block device I/O.
 *
 * Copyright (c) 2026 LG Electronics Co., Ltd.
 */

#include <linux/blkdev.h>

#include "ntfs.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
/*
 * ntfs_bdev_read - Read data directly from block device using bio
 * @bdev:	block device to read from
 * @sector:	starting sector number
 * @count:	number of bytes to read
 * @data:	destination buffer
 *
 * Reads @count bytes starting from @sector directly from the block device
 * using one or more BIOs. This function bypasses the page cache completely
 * and performs synchronous I/O with REQ_META | REQ_SYNC flags set.
 *
 * If the destination buffer @data is not a vmalloc address, it falls back
 * to the more efficient bdev_rw_virt() helper.
 */
int ntfs_bdev_read(struct block_device *bdev, sector_t sector, unsigned int count,
		 char *data)
{
	unsigned int done = 0, added;
	int error;
	struct bio *bio;
	enum req_op op;

	op = REQ_OP_READ | REQ_META | REQ_SYNC;
	if (!is_vmalloc_addr(data))
		return bdev_rw_virt(bdev, sector, data, count, op);

	bio = bio_alloc(bdev,
			bio_max_segs(DIV_ROUND_UP(count, PAGE_SIZE)),
			op, GFP_KERNEL);
	bio->bi_iter.bi_sector = sector;

	do {
		added = bio_add_vmalloc_chunk(bio, data + done, count - done);
		if (!added) {
			struct bio	*prev = bio;

			bio = bio_alloc(prev->bi_bdev,
					bio_max_segs(DIV_ROUND_UP(count - done, PAGE_SIZE)),
					prev->bi_opf, GFP_KERNEL);
			bio->bi_iter.bi_sector = bio_end_sector(prev);
			bio_chain(prev, bio);
			submit_bio(prev);
		}
		done += added;
	} while (done < count);

	error = submit_bio_wait(bio);
	bio_put(bio);

	if (op == REQ_OP_READ)
		invalidate_kernel_vmap_range(data, count);
	return error;
}
#else
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
#endif
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

#endif

/*
 * ntfs_bdev_write - Update block device contents via page cache
 * @sb:		super block of the mounted NTFS filesystem
 * @buf:	source buffer containing data to write
 * @start:	starting byte offset on the block device
 * @size:	number of bytes to write
 *
 * Writes @size bytes from @buf to the block device (sb->s_bdev) starting
 * at byte offset @start. The write is performed entirely through the page
 * cache of the block device's address space.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
int ntfs_bdev_write(struct super_block *sb, void *buf, loff_t start, loff_t size)
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
int ntfs_bdev_write(struct super_block *sb, void *buf, loff_t start, loff_t size)
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
