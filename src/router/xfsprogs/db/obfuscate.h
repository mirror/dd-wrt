// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2007, 2011 SGI
 * All Rights Reserved.
 */
#ifndef __DB_OBFUSCATE_H__
#define __DB_OBFUSCATE_H__

/* Routines to obfuscate directory filenames and xattr names. */

#define is_invalid_char(c)	((c) == '/' || (c) == '\0')

void obfuscate_name(xfs_dahash_t hash, size_t name_len, unsigned char *name,
		bool is_dirent);
int find_alternate(size_t name_len, unsigned char *name, uint32_t seq);

#endif /* __DB_OBFUSCATE_H__ */
