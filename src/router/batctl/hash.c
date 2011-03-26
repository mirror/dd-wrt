/*
 * Copyright (C) 2006-2011 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include <stdio.h>	/* NULL */
#include "hash.h"
#include "allocate.h"


/* clears the hash */
void hash_init(struct hashtable_t *hash)
{
	int i;

	hash->elements = 0;

	for (i = 0; i < hash->size; i++)
		hash->table[i] = NULL;
}

/* remove the hash structure. if hashdata_free_cb != NULL,
 * this function will be called to remove the elements inside of the hash.
 * if you don't remove the elements, memory might be leaked. */
void hash_delete(struct hashtable_t *hash, hashdata_free_cb free_cb)
{
	struct element_t *bucket, *last_bucket;
	int i;

	for (i = 0; i < hash->size; i++) {

		bucket = hash->table[i];
		while (bucket != NULL) {

			if (free_cb != NULL)
				free_cb(bucket->data);

			last_bucket = bucket;
			bucket = bucket->next;
			debugFree(last_bucket, 1301);

		}

	}

	hash_destroy(hash);
}

/* free only the hashtable and the hash itself. */
void hash_destroy(struct hashtable_t *hash)
{
	debugFree(hash->table, 1302);
	debugFree(hash, 1303);
}

/* iterate though the hash. first element is selected with iter_in NULL.
 * use the returned iterator to access the elements until hash_it_t returns NULL. */
struct hash_it_t *hash_iterate(struct hashtable_t *hash, struct hash_it_t *iter_in)
{
	struct hash_it_t *iter;

	if (iter_in == NULL) {
		iter = debugMalloc(sizeof(struct hash_it_t), 301);
		iter->index =  -1;
		iter->bucket = NULL;
		iter->prev_bucket = NULL;
	} else
		iter= iter_in;

	/* sanity checks first (if our bucket got deleted in the last iteration): */
	if (iter->bucket != NULL) {
		if (iter->first_bucket != NULL) {

			/* we're on the first element and it got removed after the last iteration. */
			if ((*iter->first_bucket) != iter->bucket) {

				/* there are still other elements in the list */
				if ((*iter->first_bucket) != NULL) {
					iter->prev_bucket = NULL;
					iter->bucket = (*iter->first_bucket);
					iter->first_bucket = &hash->table[iter->index];
					return iter;
				} else {
					iter->bucket = NULL;
				}

			}

		} else if (iter->prev_bucket != NULL) {

			/* we're not on the first element, and the bucket got removed after the last iteration.
			* the last bucket's next pointer is not pointing to our actual bucket anymore.
			* select the next. */
			if (iter->prev_bucket->next != iter->bucket)
				iter->bucket = iter->prev_bucket;

		}

	}

	/* now as we are sane, select the next one if there is some */
	if (iter->bucket != NULL) {
		if (iter->bucket->next != NULL) {
			iter->prev_bucket = iter->bucket;
			iter->bucket = iter->bucket->next;
			iter->first_bucket = NULL;
			return iter;
		}
	}
	/* if not returned yet, we've reached the last one on the index and have to search forward */

	iter->index++;
	/* go through the entries of the hash table */
	while (iter->index < hash->size) {

		if ((hash->table[iter->index]) == NULL) {
			iter->index++;
			continue;
		}

		iter->prev_bucket = NULL;
		iter->bucket = hash->table[iter->index];
		iter->first_bucket = &hash->table[iter->index];
		return iter;	/* if this table entry is not null, return it */
	}

	/* nothing to iterate over anymore */
	debugFree(iter, 1304);
	return NULL;
}

/* allocates and clears the hash */
struct hashtable_t *hash_new(int size, hashdata_compare_cb compare, hashdata_choose_cb choose)
{
	struct hashtable_t *hash;

	hash = debugMalloc(sizeof(struct hashtable_t), 302);
	if (!hash)
		return NULL;

	hash->size= size;
	hash->table= debugMalloc(sizeof(struct element_t *) * size, 303);

	if (!hash->table) {
		debugFree(hash, 1305);
		return NULL;
	}

	hash_init(hash);
	hash->compare = compare;
	hash->choose = choose;
	return hash;
}

/* adds data to the hashtable. returns 0 on success, -1 on error */
int hash_add(struct hashtable_t *hash, void *data)
{
	int index;
	struct element_t *bucket, *prev_bucket = NULL;

	index = hash->choose(data, hash->size);
	bucket = hash->table[index];

	while (bucket != NULL) {
		if (hash->compare(bucket->data, data))
			return -1;

		prev_bucket = bucket;
		bucket= bucket->next;
	}

	/* found the tail of the list, add new element */
	bucket = debugMalloc(sizeof(struct element_t),304);

	if (!bucket)
		return -1;

	/* init the new bucket */
	bucket->data = data;
	bucket->next = NULL;

	/* and link it */
	if (prev_bucket == NULL) {
		hash->table[index] = bucket;
	} else {
		prev_bucket->next = bucket;
	}

	hash->elements++;
	return 0;
}

/* finds data, based on the key in keydata. returns the found data on success, or NULL on error */
void *hash_find(struct hashtable_t *hash, void *keydata)
{
	int index;
	struct element_t *bucket;

	index = hash->choose(keydata , hash->size);
	bucket = hash->table[index];

	while (bucket != NULL) {
		if (hash->compare(bucket->data, keydata))
			return bucket->data;

		bucket = bucket->next;
	}

	return NULL;
}

/* remove bucket (this might be used in hash_iterate() if you already found the bucket
 * you want to delete and don't need the overhead to find it again with hash_remove().
 * But usually, you don't want to use this function, as it fiddles with hash-internals. */
void *hash_remove_bucket(struct hashtable_t *hash, struct hash_it_t *hash_it_t)
{
	void *data_save;

	data_save = hash_it_t->bucket->data;	/* save the pointer to the data */

	if (hash_it_t->prev_bucket != NULL) {
		hash_it_t->prev_bucket->next = hash_it_t->bucket->next;
	} else if (hash_it_t->first_bucket != NULL) {
		(*hash_it_t->first_bucket) = hash_it_t->bucket->next;
	}

	debugFree(hash_it_t->bucket, 1306);

	hash->elements--;
	return data_save;
}

/* removes data from hash, if found. returns pointer do data on success,
 * so you can remove the used structure yourself, or NULL on error .
 * data could be the structure you use with just the key filled,
 * we just need the key for comparing. */
void *hash_remove(struct hashtable_t *hash, void *data)
{
	struct hash_it_t hash_it_t;

	hash_it_t.index = hash->choose(data, hash->size);
	hash_it_t.bucket = hash->table[hash_it_t.index];
	hash_it_t.prev_bucket = NULL;

	while (hash_it_t.bucket != NULL) {
		if (hash->compare(hash_it_t.bucket->data, data)) {
			hash_it_t.first_bucket = (hash_it_t.bucket == hash->table[hash_it_t.index] ? &hash->table[ hash_it_t.index ] : NULL);
			return hash_remove_bucket(hash, &hash_it_t);
		}

		hash_it_t.prev_bucket = hash_it_t.bucket;
		hash_it_t.bucket = hash_it_t.bucket->next;
	}

	return NULL;
}

/* resize the hash, returns the pointer to the new hash or NULL on error. removes the old hash on success. */
struct hashtable_t *hash_resize(struct hashtable_t *hash, int size)
{
	struct hashtable_t *new_hash;
	struct element_t *bucket;
	int i;

	/* initialize a new hash with the new size */
	new_hash = hash_new(size, hash->compare, hash->choose);
	if (!new_hash)
		return NULL;

	/* copy the elements */
	for (i = 0; i < hash->size; i++) {
		bucket = hash->table[i];
		while (bucket != NULL) {
			hash_add(new_hash, bucket->data);
			bucket = bucket->next;
		}
	}

	hash_delete(hash, NULL);	/* remove hash and eventual overflow buckets but not the content itself. */
	return new_hash;
}

/* print the hash table for debugging */
/* void hash_debug(struct hashtable_t *hash) {
	int i;
	struct element_t *bucket;

	for (i = 0; i < hash->size; i++) {
		printf("[%d] ", i);
		bucket = hash->table[i];

		while (bucket) {
			printf("-> [%10p] ", (void *)bucket);
			bucket = bucket->next;
		}

		printf("\n");

	}
	printf("\n");
}*/

