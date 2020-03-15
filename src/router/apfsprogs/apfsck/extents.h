/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _EXTENTS_H
#define _EXTENTS_H

#include <apfs/types.h>
#include "htable.h"

struct apfs_file_extent_key;
struct apfs_dstream_id_key;
struct apfs_phys_ext_key;

/*
 * Physical extent record in memory, to be filled by extentref_lookup()
 */
struct extref_record {
	u64 phys_addr;	/* First block number */
	u64 blocks;	/* Block count */
	u64 owner;	/* Owning object id */
	u32 refcnt;	/* Reference count */
};

/*
 * Physical extent data in memory
 */
struct extent {
	struct htable_entry e_htable; /* Hash table entry header */

	u8		e_obj_type;	/* Type of the owner objects */

	/* Extent stats read from the physical extent structure */
	u32		e_refcnt;	/* Reference count */

	/* Extent stats measured by the fsck */
	u32		e_references;	/* Number of references to extent */
	u64		e_latest_owner;	/* Last owner counted on e_references */
};
#define e_bno	e_htable.h_id		/* First physical block in the extent */

/*
 * Structure used to register each physical extent for a dstream, so that the
 * references can later be counted.  The same extent structure might be shared
 * by several dstreams.
 */
struct listed_extent {
	u64			paddr;	 /* Physical address for the extent */
	struct listed_extent	*next;	 /* Next entry in linked list */
};

/*
 * Dstream data in memory
 */
struct dstream {
	struct htable_entry d_htable; /* Hash table entry header */

	/* Linked list of physical extents for dstream */
	struct listed_extent *d_extents;

	u8		d_obj_type;	/* Type of the owner objects */
	u64		d_owner;	/* Owner id for the extentref tree */
	bool		d_seen;		/* Has the dstream record been seen? */

	/* Dstream stats read from the dstream structures */
	u64		d_size;		/* Dstream size */
	u64		d_alloced_size;	/* Dstream size, including unused */
	u32		d_refcnt;	/* Reference count */

	/* Dstream stats measured by the fsck */
	u64		d_bytes;	/* Size of the extents read so far */
	u64		d_sparse_bytes;	/* Size of the holes read so far */
	u32		d_references;	/* Number of references to dstream */
};
#define d_id	d_htable.h_id		/* Dstream id */

extern void free_dstream_table(struct htable_entry **table);
extern void free_extent_table(struct htable_entry **table);
extern struct dstream *get_dstream(u64 ino);
extern void parse_extent_record(struct apfs_file_extent_key *key,
				struct apfs_file_extent_val *val, int len);
extern void parse_dstream_id_record(struct apfs_dstream_id_key *key,
				    struct apfs_dstream_id_val *val, int len);
extern u64 parse_phys_ext_record(struct apfs_phys_ext_key *key,
				 struct apfs_phys_ext_val *val, int len);

#endif	/* _EXTENTS_H */
