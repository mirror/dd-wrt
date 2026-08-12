// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Windows System Compression (WOF) decompression glue.
 *
 * Copyright (C) 2026 Hyunchul Lee <hyc.lee@gmail.com>
 */

#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "ntfs.h"
#include "inode.h"
#include "debug.h"
#include "ntfs_codec.h"
#include "attrib.h"

static const __le16 WOF_NAME[] = {
	cpu_to_le16('W'), cpu_to_le16('o'), cpu_to_le16('f'),
	cpu_to_le16('C'), cpu_to_le16('o'), cpu_to_le16('m'),
	cpu_to_le16('p'), cpu_to_le16('r'), cpu_to_le16('e'),
	cpu_to_le16('s'), cpu_to_le16('s'), cpu_to_le16('e'),
	cpu_to_le16('d'), cpu_to_le16('D'), cpu_to_le16('a'),
	cpu_to_le16('t'), cpu_to_le16('a'),
};
#define WOF_NAME_LEN 17

static int ntfs_bdev_read_from_rl(struct ntfs_volume *vol, struct runlist *runlist,
				  sector_t start_sector, u32 sector_count, void *buf)
{
	struct runlist_element *rl;
	u32 sec_per_clu_bits = vol->cluster_size_bits - 9;
	s64 vcn = start_sector >> sec_per_clu_bits;
	u32 sec_off = start_sector & ((1 << sec_per_clu_bits) - 1);
	u32 buf_off = 0;
	int err;

	down_read(&runlist->lock);
	if (!runlist->rl) {
		up_read(&runlist->lock);
		return -EINVAL;
	}

	rl = __ntfs_attr_find_vcn_nolock(runlist, vcn);
	if (IS_ERR(rl)) {
		err = PTR_ERR(rl);
		goto out_unlock;
	}

	while (sector_count > 0) {
		s64 lcn;
		loff_t byte_off;
		u32 byte_len, sectors;

		if (!rl->length || vcn < rl->vcn) {
			err = -EINVAL;
			goto out_unlock;
		}

		lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
		if (lcn < 0 && lcn != LCN_HOLE) {
			err = -EINVAL;
			goto out_unlock;
		}

		/* Sectors available in remaining clusters of this rl element */
		sectors = ((rl->vcn + rl->length - vcn) << sec_per_clu_bits) - sec_off;
		sectors = min_t(u32, sector_count, sectors);
		byte_off = ntfs_cluster_to_bytes(vol, lcn) + (sec_off << 9);
		byte_len = sectors << 9;

		if (lcn == LCN_HOLE) {
			memset((u8 *)buf + buf_off, 0, byte_len);
		} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 16, 0)
			err = ntfs_bdev_read(vol->sb->s_bdev, (char *)buf + buf_off,
					     byte_off, byte_len);
#else
			err = ntfs_dev_read(vol->sb, (char *)buf + buf_off,
					     byte_off, byte_len);
#endif
			if (err)
				goto out_unlock;
		}

		buf_off += byte_len;
		sector_count -= sectors;
		rl++;
		vcn = rl->vcn;
		sec_off = 0;
	}

	err = 0;
out_unlock:
	up_read(&runlist->lock);
	return err;
}

static int parse_wof_chunk_table(struct ntfs_inode *ni, u64 chunk_idx, u64 chunk_count,
				 u64 *chunk_offset, u32 *chunk_size)
{
	u8 bytes_per_off;
	u8 *buf_aligned = NULL, *buf = NULL;
	u64 addr[2] = {0, }, off[2];
	u64 byte_off;
	u64 chunk_data_size;
	int ret = 0;

	/* Determine offset table entry size based on file size */
	if (ni->data_size < (1ULL << 32))
		bytes_per_off = sizeof(__le32);
	else
		bytes_per_off = sizeof(__le64);

	if (!chunk_count ||
	    ni->data_size < (chunk_count - 1) * bytes_per_off)
		return -EINVAL;
	chunk_data_size = ni->data_size - (chunk_count - 1) * bytes_per_off;

	if (chunk_idx > 0)
		byte_off = (chunk_idx - 1) * bytes_per_off;
	else
		byte_off = 0;

	if (NInoNonResident(ni)) {
		sector_t start_sector;
		u32 sector_off, sectors;

		start_sector = byte_off >> 9;
		sector_off = byte_off & ((1 << 9) - 1);
		sectors = DIV_ROUND_UP(sector_off + 2 * bytes_per_off, 512);

		buf_aligned = kmalloc(sectors << 9, GFP_NOFS);
		if (buf_aligned == NULL)
			return -ENOMEM;
		if (ntfs_bdev_read_from_rl(ni->vol, &ni->runlist,
					   start_sector,
					   sectors,
					   buf_aligned)) {
			ret = -EIO;
			goto out;
		}
		buf = buf_aligned + sector_off;
	} else {
		struct ntfs_attr_search_ctx *ctx = NULL;
		u32 value_length;
		u16 value_offset;
		u8 *attr;

		ctx = ntfs_attr_get_search_ctx(ni, NULL);
		if (!ctx) {
			ret = -ENOMEM;
			goto out;
		}

		ret = ntfs_attr_lookup(AT_DATA, (__le16 *)WOF_NAME,
				       WOF_NAME_LEN, CASE_SENSITIVE, 0, NULL, 0,
				       ctx);
		if (ret) {
			ntfs_attr_put_search_ctx(ctx);
			goto out;
		}

		value_length =
			le32_to_cpu(ctx->attr->data.resident.value_length);
		value_offset =
			le16_to_cpu(ctx->attr->data.resident.value_offset);

		if (byte_off + bytes_per_off > value_length ||
		    (chunk_idx + 1 != chunk_count &&
		     byte_off + 2 * bytes_per_off > value_length)) {
			ntfs_attr_put_search_ctx(ctx);
			ret = -EINVAL;
			goto out;
		}

		attr = (u8 *)ctx->attr + value_offset;

		if (chunk_idx + 1 == chunk_count)
			memcpy((u8 *)addr, attr + byte_off, bytes_per_off);
		else
			memcpy((u8 *)addr, attr + byte_off, 2 * bytes_per_off);

		buf = (u8 *)addr;

		ntfs_attr_put_search_ctx(ctx);
	}

	/* Last chunk has an implicit end offset derived from data_size. */
	if (chunk_idx + 1 == chunk_count) {
		if (bytes_per_off == sizeof(__le32))
			((__le32 *)buf)[1] =
				cpu_to_le32(ni->data_size -
					    (chunk_count - 1) * bytes_per_off);
		else
			((__le64 *)buf)[1] =
				cpu_to_le64(ni->data_size -
					    (chunk_count - 1) * bytes_per_off);
	}

	if (bytes_per_off == sizeof(__le32)) {
		__le32 *addr = (__le32 *)buf;

		off[0] = chunk_idx ? le32_to_cpu(addr[0]) : 0;
		off[1] = chunk_idx ? le32_to_cpu(addr[1]) : le32_to_cpu(addr[0]);
	} else {
		__le64 *addr = (__le64 *)buf;

		off[0] = chunk_idx ? le64_to_cpu(addr[0]) : 0;
		off[1] = chunk_idx ? le64_to_cpu(addr[1]) : le64_to_cpu(addr[0]);
	}

	if (off[1] <= off[0] || off[1] > chunk_data_size) {
		ret = -EINVAL;
		goto out;
	}

	*chunk_offset = (chunk_count - 1) * bytes_per_off + off[0];
	*chunk_size = (u32)(off[1] - off[0]);

out:
	kfree(buf_aligned);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
int ntfs_read_wof_compressed_block(struct folio *folio)
{
	struct page *page = &folio->page;
#else
int ntfs_read_wof_compressed_block(struct page *page)
{
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	struct address_space *mapping = folio->mapping;
#else
	struct address_space *mapping = page->mapping;
#endif
	struct ntfs_inode *ni = NTFS_I(mapping->host), *wof_ni;
	struct inode *wof_inode;
	struct ntfs_volume *vol = ni->vol;
	loff_t i_size = i_size_read(VFS_I(ni));
	char *decomp_mem = NULL, *chunk_mem = NULL, *chunk_mem_aligned;
	struct page **pages = NULL;
	u32 comp_unit, pages_per_chunk, chunk_size, chunk_size_aligned, decomp_size;
	u64 chunk_offset_aligned, chunk_idx, chunk_count, chunk_offset;
	unsigned long index;
	int i, err = 0;
	const struct ntfs_codec_ops *codec;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	index = folio->index;
#else
	index = page->index;
#endif

	/* Determine frame size and frame number */
	comp_unit = 1U << ni->itype.compressed.block_size_bits;
	chunk_offset_aligned = (u64)index << PAGE_SHIFT;
	chunk_offset_aligned &= ~((u64)comp_unit - 1);
	chunk_idx = chunk_offset_aligned >> ni->itype.compressed.block_size_bits;
	chunk_count = DIV_ROUND_UP_ULL(i_size, comp_unit);

	/*
	 * If the requested page is past the end of the file, there is no
	 * chunk to decompress.  Zero the page and return success.
	 */
	if (chunk_idx >= chunk_count) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
		folio_zero_segments(folio, 0, PAGE_SIZE, 0, 0);
#else
		zero_user(page, 0, PAGE_SIZE);
#endif
		SetPageUptodate(page);
		unlock_page(page);
		return 0;
	}

	/* Select codec based on block size bits */
	switch (ni->itype.compressed.block_size_bits) {
	case 12:
		codec = &ntfs_xpress4k_codec_ops;
		break;
	case 13:
		codec = &ntfs_xpress8k_codec_ops;
		break;
	case 14:
		codec = &ntfs_xpress16k_codec_ops;
		break;
	case 15:
		codec = &ntfs_lzx32k_codec_ops;
		break;
	default:
		err = -EINVAL;
		goto out;
	}

	wof_inode = ntfs_attr_iget(VFS_I(ni), AT_DATA, (__le16 *)WOF_NAME, WOF_NAME_LEN);
	if (IS_ERR(wof_inode)) {
		err = PTR_ERR(wof_inode);
		goto out;
	}

	wof_ni = NTFS_I(wof_inode);
	if (NInoNonResident(wof_ni) && !NInoFullyMapped(wof_ni)) {
		err = ntfs_attr_map_whole_runlist(wof_ni);
		if (err)
			goto out_iput;
	}

	err = parse_wof_chunk_table(wof_ni, chunk_idx, chunk_count,
				    &chunk_offset, &chunk_size);
	if (err)
		goto out_iput;

	if (chunk_size > comp_unit) {
		ntfs_error(vol->sb, "Compressed size (%u) > frame size (%u)",
			   chunk_size, comp_unit);
		err = -EINVAL;
		goto out_iput;
	}

	if (chunk_idx + 1 == chunk_count)
		decomp_size = 1 + ((i_size - 1) & (comp_unit - 1));
	else
		decomp_size = comp_unit;

	/* Allocate pages for the uncompressed chunk */
	pages_per_chunk = comp_unit >> PAGE_SHIFT;
	pages = kcalloc(pages_per_chunk, sizeof(struct page *), GFP_NOFS);
	if (!pages) {
		err = -ENOMEM;
		goto out_iput;
	}

	for (i = 0; i < pages_per_chunk; i++) {
		unsigned long pg_index = (chunk_offset_aligned >> PAGE_SHIFT) + i;
		struct page *p;

		if (pg_index == index) {
			pages[i] = page;
			continue;
		}
		p = grab_cache_page_nowait(mapping, pg_index);
		if (!p) {
			err = -ENOMEM;
			goto out_unlock;
		}
		pages[i] = p;
	}

	decomp_mem = vmap(pages, pages_per_chunk, VM_MAP, PAGE_KERNEL);
	if (!decomp_mem) {
		err = -ENOMEM;
		goto out_unlock;
	}

	/* Allocate buffer for compressed data */
	chunk_size_aligned = DIV_ROUND_UP_ULL(chunk_offset + chunk_size, 1 << 9) -
			     (chunk_offset >> 9);
	chunk_size_aligned <<= 9;
	chunk_mem_aligned = kvmalloc(chunk_size_aligned, GFP_NOFS);
	if (!chunk_mem_aligned) {
		err = -ENOMEM;
		goto out_unmap;
	}

	/* Read compressed data from disk */
	if (!NInoNonResident(wof_ni)) {
		struct ntfs_attr_search_ctx *ctx = NULL;

		ctx = ntfs_attr_get_search_ctx(wof_ni, NULL);
		if (!ctx) {
			err = -ENOMEM;
			goto out_free;
		}

		err = ntfs_attr_lookup(AT_DATA, (__le16 *)WOF_NAME,
				       WOF_NAME_LEN, CASE_SENSITIVE,
				       0, NULL, 0, ctx);
		if (err) {
			ntfs_attr_put_search_ctx(ctx);
			goto out_free;
		}

		if (chunk_offset + chunk_size >
		    le32_to_cpu(ctx->attr->data.resident.value_length)) {
			ntfs_attr_put_search_ctx(ctx);
			err = -EINVAL;
			goto out_free;
		}

		memcpy(chunk_mem_aligned,
		       (u8 *)ctx->attr +
		       le16_to_cpu(ctx->attr->data.resident.value_offset) +
		       chunk_offset, chunk_size);
		ntfs_attr_put_search_ctx(ctx);
		chunk_mem = chunk_mem_aligned;
	} else {
		err = ntfs_bdev_read_from_rl(vol, &wof_ni->runlist,
					     chunk_offset >> 9,
					     chunk_size_aligned >> 9,
					     chunk_mem_aligned);
		if (err)
			goto out_free;
		chunk_mem = chunk_mem_aligned + (chunk_offset & ((1 << 9) - 1));
	}

	/* Decompress using codec ops with dynamic scratch */
	if (chunk_size == decomp_size) {
		memcpy(decomp_mem, chunk_mem, decomp_size);
	} else {
		void *scratch;

		scratch = kvzalloc(codec->scratch_size(comp_unit), GFP_NOFS);
		if (!scratch) {
			err = -ENOMEM;
			goto out_free;
		}
		err = codec->decompress_chunk(scratch, chunk_mem, chunk_size,
					      decomp_mem, decomp_size, comp_unit);
		kvfree(scratch);
		if (err) {
			ntfs_error(vol->sb, "Decompression failed: %d", err);
			err = -EINVAL;
			goto out_free;
		}
	}

	/* Zero any partial page at end */
	if (decomp_size < comp_unit)
		memset(decomp_mem + decomp_size, 0, comp_unit - decomp_size);

	/* Mark pages as uptodate */
	for (i = 0; i < pages_per_chunk; i++) {
		if (pages[i]) {
			SetPageUptodate(pages[i]);
			flush_dcache_page(pages[i]);
		}
	}

out_free:
	kvfree(chunk_mem_aligned);
out_unmap:
	vunmap(decomp_mem);
out_unlock:
	for (i = 0; i < pages_per_chunk; i++) {
		if (pages[i] && pages[i] != page) {
			if (err)
				ClearPageUptodate(pages[i]);
			unlock_page(pages[i]);
			put_page(pages[i]);
		}
	}
	kfree(pages);
out_iput:
	iput(wof_inode);
out:
	if (err)
		ClearPageUptodate(page);
	else
		SetPageUptodate(page);
	unlock_page(page);
	return err;
}
