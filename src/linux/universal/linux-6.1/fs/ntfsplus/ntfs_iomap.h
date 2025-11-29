/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#ifndef _LINUX_NTFS_IOMAP_H
#define _LINUX_NTFS_IOMAP_H

#include <linux/pagemap.h>
#include <linux/iomap.h>

#include "volume.h"
#include "inode.h"

extern const struct iomap_ops ntfs_write_iomap_ops;
extern const struct iomap_ops ntfs_read_iomap_ops;
extern const struct iomap_ops ntfs_page_mkwrite_iomap_ops;
extern const struct iomap_ops ntfs_dio_iomap_ops;
extern const struct iomap_writeback_ops ntfs_writeback_ops;
extern const struct iomap_folio_ops ntfs_iomap_folio_ops;
int ntfs_zeroed_clusters(struct inode *vi, s64 lcn, s64 num);
#endif /* _LINUX_NTFS_IOMAP_H */
