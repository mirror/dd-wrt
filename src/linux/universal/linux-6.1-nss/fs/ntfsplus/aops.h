/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Defines for NTFS kernel address space operations and page cache
 * handling.
 *
 * Copyright (c) 2001-2004 Anton Altaparmakov
 * Copyright (c) 2002 Richard Russon
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#ifndef _LINUX_NTFS_AOPS_H
#define _LINUX_NTFS_AOPS_H

#include <linux/pagemap.h>
#include <linux/iomap.h>

#include "volume.h"
#include "inode.h"

/**
 * ntfs_unmap_folio - release a folio that was mapped using ntfs_folio_page()
 * @folio:	the folio to release
 *
 * Unpin, unmap and release a folio that was obtained from ntfs_folio_page().
 */
static inline void ntfs_unmap_folio(struct folio *folio, void *addr)
{
	if (addr)
		kunmap_local(addr);
	folio_put(folio);
}

/**
 * ntfs_read_mapping_folio - map a folio into accessible memory, reading it if necessary
 * @mapping:	address space for which to obtain the page
 * @index:	index into the page cache for @mapping of the page to map
 *
 * Read a page from the page cache of the address space @mapping at position
 * @index, where @index is in units of PAGE_SIZE, and not in bytes.
 *
 * If the page is not in memory it is loaded from disk first using the
 * read_folio method defined in the address space operations of @mapping
 * and the page is added to the page cache of @mapping in the process.
 *
 * If the page belongs to an mst protected attribute and it is marked as such
 * in its ntfs inode (NInoMstProtected()) the mst fixups are applied but no
 * error checking is performed.  This means the caller has to verify whether
 * the ntfs record(s) contained in the page are valid or not using one of the
 * ntfs_is_XXXX_record{,p}() macros, where XXXX is the record type you are
 * expecting to see.  (For details of the macros, see fs/ntfs/layout.h.)
 *
 * If the page is in high memory it is mapped into memory directly addressible
 * by the kernel.
 *
 * Finally the page count is incremented, thus pinning the page into place.
 *
 * The above means that page_address(page) can be used on all pages obtained
 * with ntfs_map_page() to get the kernel virtual address of the page.
 *
 * When finished with the page, the caller has to call ntfs_unmap_page() to
 * unpin, unmap and release the page.
 *
 * Note this does not grant exclusive access. If such is desired, the caller
 * must provide it independently of the ntfs_{un}map_page() calls by using
 * a {rw_}semaphore or other means of serialization. A spin lock cannot be
 * used as ntfs_map_page() can block.
 *
 * The unlocked and uptodate page is returned on success or an encoded error
 * on failure. Caller has to test for error using the IS_ERR() macro on the
 * return value. If that evaluates to 'true', the negative error code can be
 * obtained using PTR_ERR() on the return value of ntfs_map_page().
 */
static inline struct folio *ntfs_read_mapping_folio(struct address_space *mapping,
		unsigned long index)
{
	struct folio *folio;

retry:
	folio = read_mapping_folio(mapping, index, NULL);
	if (PTR_ERR(folio) == -EINTR)
		goto retry;

	return folio;
}

void mark_ntfs_record_dirty(struct folio *folio);
struct bio *ntfs_setup_bio(struct ntfs_volume *vol, unsigned int opf, s64 lcn,
		unsigned int pg_ofs);
int ntfs_dev_read(struct super_block *sb, void *buf, loff_t start, loff_t end);
int ntfs_dev_write(struct super_block *sb, void *buf, loff_t start,
			loff_t size, bool wait);
#endif /* _LINUX_NTFS_AOPS_H */
