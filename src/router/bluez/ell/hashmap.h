/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_HASHMAP_H
#define __ELL_HASHMAP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*l_hashmap_foreach_func_t) (const void *key, void *value,
							void *user_data);
typedef void (*l_hashmap_destroy_func_t) (void *value);
typedef unsigned int (*l_hashmap_hash_func_t) (const void *p);
typedef int (*l_hashmap_compare_func_t) (const void *a, const void *b);
typedef void *(*l_hashmap_key_new_func_t) (const void *p);
typedef void (*l_hashmap_key_free_func_t) (void *p);
typedef bool (*l_hashmap_remove_func_t)(const void *key, void *value,
						void *user_data);

struct l_hashmap;

unsigned int l_str_hash(const void *p);

struct l_hashmap *l_hashmap_new(void);
struct l_hashmap *l_hashmap_string_new(void);

bool l_hashmap_set_hash_function(struct l_hashmap *hashmap,
						l_hashmap_hash_func_t func);
bool l_hashmap_set_compare_function(struct l_hashmap *hashmap,
						l_hashmap_compare_func_t func);
bool l_hashmap_set_key_copy_function(struct l_hashmap *hashmap,
						l_hashmap_key_new_func_t func);
bool l_hashmap_set_key_free_function(struct l_hashmap *hashmap,
					l_hashmap_key_free_func_t func);

void l_hashmap_destroy(struct l_hashmap *hashmap,
			l_hashmap_destroy_func_t destroy);

bool l_hashmap_insert(struct l_hashmap *hashmap,
			const void *key, void *value);
bool l_hashmap_replace(struct l_hashmap *hashmap,
					const void *key, void *value,
					void **old_value);
void *l_hashmap_remove(struct l_hashmap *hashmap, const void *key);
void *l_hashmap_lookup(struct l_hashmap *hashmap, const void *key);

void l_hashmap_foreach(struct l_hashmap *hashmap,
			l_hashmap_foreach_func_t function, void *user_data);
unsigned int l_hashmap_foreach_remove(struct l_hashmap *hashmap,
			l_hashmap_remove_func_t function, void *user_data);

unsigned int l_hashmap_size(struct l_hashmap *hashmap);
bool l_hashmap_isempty(struct l_hashmap *hashmap);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_HASHMAP_H */
