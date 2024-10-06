/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdlib.h>
#include <sys/mman.h>
#include <apfs/checksum.h>
#include <apfs/raw.h>
#include "mkapfs.h"
#include "object.h"

/**
 * set_object_header - Set the header for a filesystem object
 * @obj:	pointer to the on-disk object header
 * @size:	size of the object (in bytes)
 * @oid:	object id
 * @type:	object type
 * @subtype:	object subtype
 *
 * All other fields of the object headed by @obj must be set in advance by
 * the caller, otherwise the checksum won't be correct.
 */
void set_object_header(struct apfs_obj_phys *obj, u32 size, u64 oid, u32 type, u32 subtype)
{
	char *after_cksum = (char *)obj + APFS_MAX_CKSUM_SIZE;
	int after_cksum_len = size - APFS_MAX_CKSUM_SIZE;

	obj->o_oid = cpu_to_le64(oid);
	obj->o_xid = cpu_to_le64(MKFS_XID);
	obj->o_type = cpu_to_le32(type);
	obj->o_subtype = cpu_to_le32(subtype);

	obj->o_cksum = cpu_to_le64(fletcher64(after_cksum, after_cksum_len));
}
