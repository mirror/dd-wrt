// SPDX-License-Identifier: GPL-2.0
/*
 *
 * Copyright (C) 2019-2020 Paragon Software GmbH, All rights reserved.
 *
 */
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/module.h>
#include <linux/nls.h>

#include "debug.h"
#include "ntfs.h"
#include "ntfs_fs.h"

static inline u16 upcase_unicode_char(const u16 *upcase, u16 chr)
{
	if (chr < 'a')
		return chr;

	if (chr <= 'z')
		return chr - ('a' - 'A');

	return upcase[chr];
}

int ntfs_cmp_names(const __le16 *s1, size_t l1, const __le16 *s2, size_t l2,
		   const u16 *upcase)
{
	int diff;
	size_t len = l1 < l2 ? l1 : l2;

	if (upcase) {
		while (len--) {
			diff = upcase_unicode_char(upcase, le16_to_cpu(*s1++)) -
			       upcase_unicode_char(upcase, le16_to_cpu(*s2++));
			if (diff)
				return diff;
		}
	} else {
		while (len--) {
			diff = le16_to_cpu(*s1++) - le16_to_cpu(*s2++);
			if (diff)
				return diff;
		}
	}

	return (int)(l1 - l2);
}

int ntfs_cmp_names_cpu(const struct cpu_str *uni1, const struct le_str *uni2,
		       const u16 *upcase)
{
	const u16 *s1 = uni1->name;
	const __le16 *s2 = uni2->name;
	size_t l1 = uni1->len;
	size_t l2 = uni2->len;
	size_t len = l1 < l2 ? l1 : l2;
	int diff;

	if (upcase) {
		while (len--) {
			diff = upcase_unicode_char(upcase, *s1++) -
			       upcase_unicode_char(upcase, le16_to_cpu(*s2++));
			if (diff)
				return diff;
		}
	} else {
		while (len--) {
			diff = *s1++ - le16_to_cpu(*s2++);
			if (diff)
				return diff;
		}
	}

	return l1 - l2;
}
