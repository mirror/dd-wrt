/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _HTABLE_H
#define _HTABLE_H

#include <apfs/types.h>

#define HTABLE_BUCKETS	512	/* So the hash table array fits in 4k */

/*
 * Structure of the common header for hash table entries
 */
struct htable_entry {
	struct htable_entry	*h_next;	/* Next entry in linked list */
	u64			h_id;		/* Catalog object id of entry */
};

/* State of the in-memory listed cnid structure */
#define CNID_UNUSED		0 /* The cnid is unused */
#define CNID_IN_SIBLING_LINK	1 /* The cnid was seen in a sibling link */
#define CNID_IN_INODE		2 /* The cnid was seen in an inode */
#define CNID_IN_DSTREAM		4 /* The cnid was seen in a dstream */

/*
 * Structure used to register each catalog node id (cnid) that has been seen,
 * and check that they are not repeated.
 */
struct listed_cnid {
	struct htable_entry	c_htable;	/* Hash table entry header */
	u8			c_state;
};

static inline void cnid_set_state_flag(struct listed_cnid *cnid, u8 flag)
{
	if (cnid->c_state & flag)
		report("Catalog", "a filesystem object id was used twice.");
	cnid->c_state |= flag;
}

extern struct htable_entry **alloc_htable();
extern void free_htable(struct htable_entry **table,
			void (*free_entry)(struct htable_entry *));
extern void apply_on_htable(struct htable_entry **table, void (*fn)(struct htable_entry *));
extern struct htable_entry *get_htable_entry(u64 id, int size,
					     struct htable_entry **table);
extern bool htable_entry_exists(u64 id, struct htable_entry **table);
extern void free_cnid_table(struct htable_entry **table);
extern struct listed_cnid *get_listed_cnid(u64 id);

#endif	/* _HTABLE_H */
