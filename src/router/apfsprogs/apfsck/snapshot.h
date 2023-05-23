/*
 * Copyright (C) 2022 Ernesto A. Fernández <ernesto@corellium.com>
 */

#ifndef _SNAPSHOT_H
#define _SNAPSHOT_H

#include <apfs/types.h>
#include "htable.h"

/*
 * Snapshot data in memory
 */
struct snapshot {
	struct htable_entry sn_htable; /* Hash table entry header */

	bool sn_name_seen;	/* Has the snap's name been seen? */
	bool sn_meta_seen;	/* Has the snap's metadata been seen? */
	bool sn_omap_seen;	/* Is the snap in the omap snap tree? */
	char *sn_meta_name;	/* Name reported by the metadata rec */
};
#define sn_xid	sn_htable.h_id	/* Transaction id */

extern void free_snap_table(struct htable_entry **table);
extern struct snapshot *get_snapshot(u64 xid);
extern void parse_snap_record(void *key, void *val, int len);
extern void parse_omap_snap_record(__le64 *key, struct apfs_omap_snapshot *val, int len);

#endif	/* _SNAPSHOT_H */
