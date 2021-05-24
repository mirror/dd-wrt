/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _DIR_H
#define _DIR_H

#include <apfs/types.h>

extern void parse_dentry_record(void *key, struct apfs_drec_val *val, int len);

#endif	/* _DIR_H */
