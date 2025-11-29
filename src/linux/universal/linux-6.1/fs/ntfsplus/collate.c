// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * NTFS kernel collation handling. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2004 Anton Altaparmakov
 *
 * Part of this file is based on code from the NTFS-3G project.
 * and is copyrighted by the respective authors below:
 * Copyright (c) 2004 Anton Altaparmakov
 * Copyright (c) 2005 Yura Pakhuchiy
 */

#include "collate.h"
#include "misc.h"
#include "ntfs.h"

static int ntfs_collate_binary(struct ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;

	ntfs_debug("Entering.");
	rc = memcmp(data1, data2, min(data1_len, data2_len));
	if (!rc && (data1_len != data2_len)) {
		if (data1_len < data2_len)
			rc = -1;
		else
			rc = 1;
	}
	ntfs_debug("Done, returning %i", rc);
	return rc;
}

static int ntfs_collate_ntofs_ulong(struct ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;
	u32 d1, d2;

	ntfs_debug("Entering.");

	if (data1_len != data2_len || data1_len != 4)
		return -EINVAL;

	d1 = le32_to_cpup(data1);
	d2 = le32_to_cpup(data2);
	if (d1 < d2)
		rc = -1;
	else {
		if (d1 == d2)
			rc = 0;
		else
			rc = 1;
	}
	ntfs_debug("Done, returning %i", rc);
	return rc;
}

/**
 * ntfs_collate_ntofs_ulongs - Which of two le32 arrays should be listed first
 *
 * Returns: -1, 0 or 1 depending of how the arrays compare
 */
static int ntfs_collate_ntofs_ulongs(struct ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;
	int len;
	const __le32 *p1, *p2;
	u32 d1, d2;

	ntfs_debug("Entering.");
	if ((data1_len != data2_len) || (data1_len <= 0) || (data1_len & 3)) {
		ntfs_error(vol->sb, "data1_len or data2_len not valid\n");
		return -1;
	}

	p1 = (const __le32 *)data1;
	p2 = (const __le32 *)data2;
	len = data1_len;
	do {
		d1 = le32_to_cpup(p1);
		p1++;
		d2 = le32_to_cpup(p2);
		p2++;
	} while ((d1 == d2) && ((len -= 4) > 0));
	if (d1 < d2)
		rc = -1;
	else {
		if (d1 == d2)
			rc = 0;
		else
			rc = 1;
	}
	ntfs_debug("Done, returning %i.", rc);
	return rc;
}

/**
 * ntfs_collate_file_name - Which of two filenames should be listed first
 */
static int ntfs_collate_file_name(struct ntfs_volume *vol,
		const void *data1, const int __always_unused data1_len,
		const void *data2, const int __always_unused data2_len)
{
	int rc;

	ntfs_debug("Entering.\n");
	rc = ntfs_file_compare_values(data1, data2, -2,
			IGNORE_CASE, vol->upcase, vol->upcase_len);
	if (!rc)
		rc = ntfs_file_compare_values(data1, data2,
			-2, CASE_SENSITIVE, vol->upcase, vol->upcase_len);
	ntfs_debug("Done, returning %i.\n", rc);
	return rc;
}

typedef int (*ntfs_collate_func_t)(struct ntfs_volume *, const void *, const int,
		const void *, const int);

static ntfs_collate_func_t ntfs_do_collate0x0[3] = {
	ntfs_collate_binary,
	ntfs_collate_file_name,
	NULL/*ntfs_collate_unicode_string*/,
};

static ntfs_collate_func_t ntfs_do_collate0x1[4] = {
	ntfs_collate_ntofs_ulong,
	NULL/*ntfs_collate_ntofs_sid*/,
	NULL/*ntfs_collate_ntofs_security_hash*/,
	ntfs_collate_ntofs_ulongs,
};

/**
 * ntfs_collate - collate two data items using a specified collation rule
 * @vol:	ntfs volume to which the data items belong
 * @cr:		collation rule to use when comparing the items
 * @data1:	first data item to collate
 * @data1_len:	length in bytes of @data1
 * @data2:	second data item to collate
 * @data2_len:	length in bytes of @data2
 *
 * Collate the two data items @data1 and @data2 using the collation rule @cr
 * and return -1, 0, ir 1 if @data1 is found, respectively, to collate before,
 * to match, or to collate after @data2.
 *
 * For speed we use the collation rule @cr as an index into two tables of
 * function pointers to call the appropriate collation function.
 */
int ntfs_collate(struct ntfs_volume *vol, __le32 cr,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int i;

	ntfs_debug("Entering.");

	if (cr != COLLATION_BINARY && cr != COLLATION_NTOFS_ULONG &&
	    cr != COLLATION_FILE_NAME && cr != COLLATION_NTOFS_ULONGS)
		return -EINVAL;

	i = le32_to_cpu(cr);
	if (i < 0)
		return -1;
	if (i <= 0x02)
		return ntfs_do_collate0x0[i](vol, data1, data1_len,
				data2, data2_len);
	if (i < 0x10)
		return -1;
	i -= 0x10;
	if (likely(i <= 3))
		return ntfs_do_collate0x1[i](vol, data1, data1_len,
				data2, data2_len);
	return 0;
}
