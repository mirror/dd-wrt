/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <apfs/types.h>

struct apfs_obj_phys;

extern void set_object_header(struct apfs_obj_phys *obj, u32 size, u64 oid, u32 type, u32 subtype);

#endif	/* _OBJECT_H */
