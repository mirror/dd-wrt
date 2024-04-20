/**
 * @file hash_table.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang hash table
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_HASH_TABLE_H_
#define LY_HASH_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "log.h"

/**
 * @struct ly_ht
 * @brief libyang hash table.
 */
struct ly_ht;

/**
 * @brief Compute hash from (several) string(s).
 *
 * Usage:
 * - init hash to 0
 * - repeatedly call ::lyht_hash_multi(), provide hash from the last call
 * - call ::lyht_hash_multi() with key_part = NULL to finish the hash
 *
 * @param[in] hash Previous hash.
 * @param[in] key_part Next key to hash,
 * @param[in] len Length of @p key_part.
 * @return Hash with the next key.
 */
LIBYANG_API_DECL uint32_t lyht_hash_multi(uint32_t hash, const char *key_part, size_t len);

/**
 * @brief Compute hash from a string.
 *
 * Bob Jenkin's one-at-a-time hash
 * http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Spooky hash is faster, but it works only for little endian architectures.
 *
 * @param[in] key Key to hash.
 * @param[in] len Length of @p key.
 * @return Hash of the key.
 */
LIBYANG_API_DECL uint32_t lyht_hash(const char *key, size_t len);

/**
 * @brief Callback for checking hash table values equivalence.
 *
 * @param[in] val1_p Pointer to the first value, the one being searched (inserted/removed).
 * @param[in] val2_p Pointer to the second value, the one stored in the hash table.
 * @param[in] mod Whether the operation modifies the hash table (insert or remove) or not (find).
 * @param[in] cb_data User callback data.
 * @return false (non-equal) or true (equal).
 */
typedef ly_bool (*lyht_value_equal_cb)(void *val1_p, void *val2_p, ly_bool mod, void *cb_data);

/**
 * @brief Create new hash table.
 *
 * @param[in] size Starting size of the hash table (capacity of values), must be power of 2.
 * @param[in] val_size Size in bytes of value (the stored hashed item).
 * @param[in] val_equal Callback for checking value equivalence.
 * @param[in] cb_data User data always passed to @p val_equal.
 * @param[in] resize Whether to resize the table on too few/too many records taken.
 * @return Empty hash table, NULL on error.
 */
LIBYANG_API_DECL struct ly_ht *lyht_new(uint32_t size, uint16_t val_size, lyht_value_equal_cb val_equal, void *cb_data,
        uint16_t resize);

/**
 * @brief Set hash table value equal callback.
 *
 * @param[in] ht Hash table to modify.
 * @param[in] new_val_equal New callback for checking value equivalence.
 * @return Previous callback for checking value equivalence.
 */
LIBYANG_API_DECL lyht_value_equal_cb lyht_set_cb(struct ly_ht *ht, lyht_value_equal_cb new_val_equal);

/**
 * @brief Set hash table value equal callback user data.
 *
 * @param[in] ht Hash table to modify.
 * @param[in] new_cb_data New data for values callback.
 * @return Previous data for values callback.
 */
LIBYANG_API_DECL void *lyht_set_cb_data(struct ly_ht *ht, void *new_cb_data);

/**
 * @brief Make a duplicate of an existing hash table.
 *
 * @param[in] orig Original hash table to duplicate.
 * @return Duplicated hash table @p orig, NULL on error.
 */
LIBYANG_API_DECL struct ly_ht *lyht_dup(const struct ly_ht *orig);

/**
 * @brief Free a hash table.
 *
 * @param[in] ht Hash table to be freed.
 * @param[in] val_free Optional callback for freeing all the stored values, @p val_p is a pointer to a stored value.
 */
LIBYANG_API_DECL void lyht_free(struct ly_ht *ht, void (*val_free)(void *val_p));

/**
 * @brief Find a value in a hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the value to find.
 * @param[in] hash Hash of the stored value.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LIBYANG_API_DECL LY_ERR lyht_find(const struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Find a value in a hash table but use a custom val_equal callback.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the value to find.
 * @param[in] hash Hash of the stored value.
 * @param[in] val_equal Callback for checking value equivalence.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LIBYANG_API_DECL LY_ERR lyht_find_with_val_cb(const struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb val_equal, void **match_p);

/**
 * @brief Find another equal value in the hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the previously found value in @p ht.
 * @param[in] hash Hash of the previously found value.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LIBYANG_API_DECL LY_ERR lyht_find_next(const struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Find another equal value in the hash table. Same functionality as ::lyht_find_next()
 * but allows to specify a collision val equal callback to be used for checking for matching colliding values.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the previously found value in @p ht.
 * @param[in] hash Hash of the previously found value.
 * @param[in] collision_val_equal Val equal callback to use for checking collisions.
 * @param[out] match_p Pointer to the matching value, optional.
 * @return LY_SUCCESS if value was found,
 * @return LY_ENOTFOUND if not found.
 */
LIBYANG_API_DECL LY_ERR lyht_find_next_with_collision_cb(const struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb collision_val_equal, void **match_p);

/**
 * @brief Insert a value into a hash table.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] val_p Pointer to the value to insert. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[out] match_p Pointer to the stored value, optional
 * @return LY_SUCCESS on success,
 * @return LY_EEXIST in case the value is already present.
 * @return LY_EMEM in case of memory allocation failure.
 */
LIBYANG_API_DECL LY_ERR lyht_insert(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Insert a value into a hash table, without checking whether the value has already been inserted.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] val_p Pointer to the value to insert. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[out] match_p Pointer to the stored value, optional
 * @return LY_SUCCESS on success,
 * @return LY_EMEM in case of memory allocation failure.
 */
LIBYANG_API_DECL LY_ERR lyht_insert_no_check(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p);

/**
 * @brief Insert a value into hash table. Same functionality as ::lyht_insert()
 * but allows to specify a temporary val equal callback to be used in case the hash table
 * will be resized after successful insertion.
 *
 * @param[in] ht Hash table to insert into.
 * @param[in] val_p Pointer to the value to insert. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[in] resize_val_equal Val equal callback to use for resizing.
 * @param[out] match_p Pointer to the stored value, optional
 * @return LY_SUCCESS on success,
 * @return LY_EEXIST in case the value is already present.
 * @return LY_EMEM in case of memory allocation failure.
 */
LIBYANG_API_DECL LY_ERR lyht_insert_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb resize_val_equal, void **match_p);

/**
 * @brief Remove a value from a hash table.
 *
 * @param[in] ht Hash table to remove from.
 * @param[in] val_p Pointer to value to be removed. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @return LY_SUCCESS on success,
 * @return LY_ENOTFOUND if value was not found.
 */
LIBYANG_API_DECL LY_ERR lyht_remove(struct ly_ht *ht, void *val_p, uint32_t hash);

/**
 * @brief Remove a value from a hash table. Same functionality as ::lyht_remove()
 * but allows to specify a temporary val equal callback to be used in case the hash table
 * will be resized after successful removal.
 *
 * @param[in] ht Hash table to remove from.
 * @param[in] val_p Pointer to value to be removed. Be careful, if the values stored in the hash table
 * are pointers, @p val_p must be a pointer to a pointer.
 * @param[in] hash Hash of the stored value.
 * @param[in] resize_val_equal Val equal callback to use for resizing.
 * @return LY_SUCCESS on success,
 * @return LY_ENOTFOUND if value was not found.
 */
LIBYANG_API_DECL LY_ERR lyht_remove_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb resize_val_equal);

/**
 * @brief Get suitable size of a hash table for a fixed number of items.
 *
 * @param[in] item_count Number of stored items.
 * @return Hash table size.
 */
LIBYANG_API_DECL uint32_t lyht_get_fixed_size(uint32_t item_count);

#ifdef __cplusplus
}
#endif

#endif /* LY_HASH_TABLE_H_ */
