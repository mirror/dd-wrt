/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _DIR_H
#define _DIR_H

#include <apfs/types.h>
#include "htable.h"

/*
 * Directory stats in memory
 */
struct dirstat {
	struct htable_entry ds_htable; /* Hash table entry header */

	bool		ds_seen;	/* Has the record been seen? */
	bool		ds_origin_seen;	/* Has the origin inode been seen? */

	/* Directory stats read from the dir stats record */
	u64		ds_num_children;/* Number of files and folders inside */
	u64		ds_total_size;	/* Total size of all files inside */
	u64		ds_chained_key;	/* TODO: figure this out */

	/* Directory stats measured by the fsck */
	u64		ds_child_count;	/* Number of files and folders inside */
	u64		ds_child_size;	/* Total size of all files inside */
	u64		ds_origin;	/* Id of owning inode */
};
#define ds_oid	ds_htable.h_id		/* Object id */

extern void parse_dentry_record(void *key, struct apfs_drec_val *val, int len);
extern void parse_dir_stats_record(void *key, struct apfs_dir_stats_val *val, int len);
extern struct dirstat *get_dirstat(u64 oid);
extern void free_dirstat_table(struct htable_entry **table);

#endif	/* _DIR_H */
