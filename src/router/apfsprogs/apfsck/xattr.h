/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _XATTR_H
#define _XATTR_H

#include <apfs/types.h>
#include "inode.h"

struct apfs_xattr_key;

extern void parse_xattr_record(struct apfs_xattr_key *key,
			       struct apfs_xattr_val *val, int len);

#endif	/* _XATTR_H */
