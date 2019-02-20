#ifndef BASE_DATA_STRUCT_HASH_H
#define BASE_DATA_STRUCT_HASH_H

#include <stdint.h>

//----------------------------------------------------------------

struct dm_hash_table;
struct dm_hash_node;

typedef void (*dm_hash_iterate_fn) (void *data);

struct dm_hash_table *dm_hash_create(unsigned size_hint)
	__attribute__((__warn_unused_result__));
void dm_hash_destroy(struct dm_hash_table *t);
void dm_hash_wipe(struct dm_hash_table *t);

void *dm_hash_lookup(struct dm_hash_table *t, const char *key);
int dm_hash_insert(struct dm_hash_table *t, const char *key, void *data);
void dm_hash_remove(struct dm_hash_table *t, const char *key);

void *dm_hash_lookup_binary(struct dm_hash_table *t, const void *key, uint32_t len);
int dm_hash_insert_binary(struct dm_hash_table *t, const void *key, uint32_t len,
			  void *data);
void dm_hash_remove_binary(struct dm_hash_table *t, const void *key, uint32_t len);

unsigned dm_hash_get_num_entries(struct dm_hash_table *t);
void dm_hash_iter(struct dm_hash_table *t, dm_hash_iterate_fn f);

char *dm_hash_get_key(struct dm_hash_table *t, struct dm_hash_node *n);
void *dm_hash_get_data(struct dm_hash_table *t, struct dm_hash_node *n);
struct dm_hash_node *dm_hash_get_first(struct dm_hash_table *t);
struct dm_hash_node *dm_hash_get_next(struct dm_hash_table *t, struct dm_hash_node *n);

/*
 * dm_hash_insert() replaces the value of an existing
 * entry with a matching key if one exists.  Otherwise
 * it adds a new entry.
 *
 * dm_hash_insert_with_val() inserts a new entry if
 * another entry with the same key already exists.
 * val_len is the size of the data being inserted.
 *
 * If two entries with the same key exist,
 * (added using dm_hash_insert_allow_multiple), then:
 * . dm_hash_lookup() returns the first one it finds, and
 *   dm_hash_lookup_with_val() returns the one with a matching
 *   val_len/val.
 * . dm_hash_remove() removes the first one it finds, and
 *   dm_hash_remove_with_val() removes the one with a matching
 *   val_len/val.
 *
 * If a single entry with a given key exists, and it has
 * zero val_len, then:
 * . dm_hash_lookup() returns it
 * . dm_hash_lookup_with_val(val_len=0) returns it
 * . dm_hash_remove() removes it
 * . dm_hash_remove_with_val(val_len=0) removes it
 *
 * dm_hash_lookup_with_count() is a single call that will
 * both lookup a key's value and check if there is more
 * than one entry with the given key.
 *
 * (It is not meant to retrieve all the entries with the
 * given key.  In the common case where a single entry exists
 * for the key, it is useful to have a single call that will
 * both look up the value and indicate if multiple values
 * exist for the key.)
 *
 * dm_hash_lookup_with_count:
 * . If no entries exist, the function returns NULL, and
 *   the count is set to 0.
 * . If only one entry exists, the value of that entry is
 *   returned and count is set to 1.
 * . If N entries exists, the value of the first entry is
 *   returned and count is set to N.
 */

void *dm_hash_lookup_with_val(struct dm_hash_table *t, const char *key,
                              const void *val, uint32_t val_len);
void dm_hash_remove_with_val(struct dm_hash_table *t, const char *key,
                             const void *val, uint32_t val_len);
int dm_hash_insert_allow_multiple(struct dm_hash_table *t, const char *key,
                                  const void *val, uint32_t val_len);
void *dm_hash_lookup_with_count(struct dm_hash_table *t, const char *key, int *count);


#define dm_hash_iterate(v, h) \
	for (v = dm_hash_get_first((h)); v; \
	     v = dm_hash_get_next((h), v))

//----------------------------------------------------------------

#endif
