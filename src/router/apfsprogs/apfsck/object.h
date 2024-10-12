/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <apfs/types.h>
#include "htable.h"

struct apfs_obj_phys;
struct super_block;
struct node;

/*
 * In-memory representation of an APFS object
 */
struct object {
	u64 block_nr;
	u64 oid;	/* Often the same as the block number */
	u64 xid;
	u32 type;
	u32 subtype;
	u32 flags;

	/*
	 * Size of the object in bytes. It's usually a single block, the only
	 * exception I've found so far are the spacemans of containers that are
	 * too big to have all cib addresses in one block, but too small to fill
	 * a whole cab with cib addresses.
	 */
	u32 size;
};

extern int obj_verify_csum(struct apfs_obj_phys *obj);
extern void *read_object_nocheck(u64 bno, u32 size, struct object *obj);
extern u32 parse_object_flags(u32 flags, bool encrypted);
extern void *read_object(u64 oid, struct htable_entry **omap_table, struct object *obj);
extern void *read_object_noheader(u64 oid, struct htable_entry **omap_table, struct object *obj);
extern void free_cpoint_map_table(struct htable_entry **table);
extern struct cpoint_map *get_cpoint_map(u64 oid);
extern void *read_ephemeral_object(u64 oid, struct object *obj);

#endif	/* _OBJECT_H */
