/**
 * @file hash_table.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang generic hash table implementation
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "hash_table.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "log.h"
#include "ly_common.h"

LIBYANG_API_DEF uint32_t
lyht_hash_multi(uint32_t hash, const char *key_part, size_t len)
{
    uint32_t i;

    if (key_part && len) {
        for (i = 0; i < len; ++i) {
            hash += key_part[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
    } else {
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
    }

    return hash;
}

LIBYANG_API_DEF uint32_t
lyht_hash(const char *key, size_t len)
{
    uint32_t hash;

    hash = lyht_hash_multi(0, key, len);
    return lyht_hash_multi(hash, NULL, len);
}

static LY_ERR
lyht_init_hlists_and_records(struct ly_ht *ht)
{
    struct ly_ht_rec *rec;
    uint32_t i;

    ht->recs = calloc(ht->size, ht->rec_size);
    LY_CHECK_ERR_RET(!ht->recs, LOGMEM(NULL), LY_EMEM);
    for (i = 0; i < ht->size; i++) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        if (i != ht->size) {
            rec->next = i + 1;
        } else {
            rec->next = LYHT_NO_RECORD;
        }
    }

    ht->hlists = malloc(sizeof(ht->hlists[0]) * ht->size);
    LY_CHECK_ERR_RET(!ht->hlists, free(ht->recs); LOGMEM(NULL), LY_EMEM);
    for (i = 0; i < ht->size; i++) {
        ht->hlists[i].first = LYHT_NO_RECORD;
        ht->hlists[i].last = LYHT_NO_RECORD;
    }
    ht->first_free_rec = 0;

    return LY_SUCCESS;
}

LIBYANG_API_DEF struct ly_ht *
lyht_new(uint32_t size, uint16_t val_size, lyht_value_equal_cb val_equal, void *cb_data, uint16_t resize)
{
    struct ly_ht *ht;

    /* check that 2^x == size (power of 2) */
    assert(size && !(size & (size - 1)));
    assert(val_equal && val_size);
    assert(resize == 0 || resize == 1);

    if (size < LYHT_MIN_SIZE) {
        size = LYHT_MIN_SIZE;
    }

    ht = malloc(sizeof *ht);
    LY_CHECK_ERR_RET(!ht, LOGMEM(NULL), NULL);

    ht->used = 0;
    ht->size = size;
    ht->val_equal = val_equal;
    ht->cb_data = cb_data;
    ht->resize = resize;

    ht->rec_size = SIZEOF_LY_HT_REC + val_size;
    if (lyht_init_hlists_and_records(ht) != LY_SUCCESS) {
        free(ht);
        return NULL;
    }

    return ht;
}

LIBYANG_API_DEF lyht_value_equal_cb
lyht_set_cb(struct ly_ht *ht, lyht_value_equal_cb new_val_equal)
{
    lyht_value_equal_cb prev;

    prev = ht->val_equal;
    ht->val_equal = new_val_equal;
    return prev;
}

LIBYANG_API_DEF void *
lyht_set_cb_data(struct ly_ht *ht, void *new_cb_data)
{
    void *prev;

    prev = ht->cb_data;
    ht->cb_data = new_cb_data;
    return prev;
}

LIBYANG_API_DEF struct ly_ht *
lyht_dup(const struct ly_ht *orig)
{
    struct ly_ht *ht;

    LY_CHECK_ARG_RET(NULL, orig, NULL);

    ht = lyht_new(orig->size, orig->rec_size - SIZEOF_LY_HT_REC, orig->val_equal, orig->cb_data, orig->resize ? 1 : 0);
    if (!ht) {
        return NULL;
    }

    memcpy(ht->hlists, orig->hlists, sizeof(ht->hlists[0]) * orig->size);
    memcpy(ht->recs, orig->recs, (size_t)orig->size * orig->rec_size);
    ht->used = orig->used;
    return ht;
}

LIBYANG_API_DEF void
lyht_free(struct ly_ht *ht, void (*val_free)(void *val_p))
{
    struct ly_ht_rec *rec;
    uint32_t hlist_idx;
    uint32_t rec_idx;

    if (!ht) {
        return;
    }

    if (val_free) {
        LYHT_ITER_ALL_RECS(ht, hlist_idx, rec_idx, rec) {
            val_free(&rec->val);
        }
    }
    free(ht->hlists);
    free(ht->recs);
    free(ht);
}

/**
 * @brief Resize a hash table.
 *
 * @param[in] ht Hash table to resize.
 * @param[in] operation Operation to perform. 1 to enlarge, -1 to shrink, 0 to only rehash all records.
 * @return LY_ERR value.
 */
static LY_ERR
lyht_resize(struct ly_ht *ht, int operation, int check)
{
    struct ly_ht_rec *rec;
    struct ly_ht_hlist *old_hlists;
    unsigned char *old_recs;
    uint32_t old_first_free_rec;
    uint32_t i, old_size;
    uint32_t rec_idx;

    old_hlists = ht->hlists;
    old_recs = ht->recs;
    old_size = ht->size;
    old_first_free_rec = ht->first_free_rec;

    if (operation > 0) {
        /* double the size */
        ht->size <<= 1;
    } else if (operation < 0) {
        /* half the size */
        ht->size >>= 1;
    }

    if (lyht_init_hlists_and_records(ht) != LY_SUCCESS) {
        ht->hlists = old_hlists;
        ht->recs = old_recs;
        ht->size = old_size;
        ht->first_free_rec = old_first_free_rec;
        return LY_EMEM;
    }

    /* reset used, it will increase again */
    ht->used = 0;

    /* add all the old records into the new records array */
    for (i = 0; i < old_size; i++) {
        for (rec_idx = old_hlists[i].first, rec = lyht_get_rec(old_recs, ht->rec_size, rec_idx);
                rec_idx != LYHT_NO_RECORD;
                rec_idx = rec->next, rec = lyht_get_rec(old_recs, ht->rec_size, rec_idx)) {
            LY_ERR ret;

            if (check) {
                ret = lyht_insert(ht, rec->val, rec->hash, NULL);
            } else {
                ret = lyht_insert_no_check(ht, rec->val, rec->hash, NULL);
            }

            assert(!ret);
            (void)ret;
        }
    }

    /* final touches */
    free(old_recs);
    free(old_hlists);
    return LY_SUCCESS;
}

/**
 * @brief Search for a record with specific value and hash.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the value to find.
 * @param[in] hash Hash to find.
 * @param[in] mod Whether the operation modifies the hash table (insert or remove) or not (find).
 * @param[in] val_equal Callback for checking value equivalence.
 * @param[out] crec_p Optional found first record.
 * @param[out] col Optional collision number of @p rec_p, 0 for no collision.
 * @param[out] rec_p Found exact matching record, may be a collision of @p crec_p.
 * @return LY_ENOTFOUND if no record found,
 * @return LY_SUCCESS if record was found.
 */
static LY_ERR
lyht_find_rec(const struct ly_ht *ht, void *val_p, uint32_t hash, ly_bool mod, lyht_value_equal_cb val_equal,
        struct ly_ht_rec **crec_p, uint32_t *col, struct ly_ht_rec **rec_p)
{
    uint32_t hlist_idx = hash & (ht->size - 1);
    struct ly_ht_rec *rec;
    uint32_t rec_idx;

    if (crec_p) {
        *crec_p = NULL;
    }
    if (col) {
        *col = 0;
    }
    *rec_p = NULL;

    LYHT_ITER_HLIST_RECS(ht, hlist_idx, rec_idx, rec) {
        if ((rec->hash == hash) && val_equal(val_p, &rec->val, mod, ht->cb_data)) {
            if (crec_p) {
                *crec_p = rec;
            }
            *rec_p = rec;
            return LY_SUCCESS;
        }

        if (col) {
            *col = *col + 1;
        }
    }

    /* not found even in collisions */
    return LY_ENOTFOUND;
}

LIBYANG_API_DEF LY_ERR
lyht_find(const struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p)
{
    struct ly_ht_rec *rec;

    lyht_find_rec(ht, val_p, hash, 0, ht->val_equal, NULL, NULL, &rec);

    if (rec && match_p) {
        *match_p = rec->val;
    }
    return rec ? LY_SUCCESS : LY_ENOTFOUND;
}

LIBYANG_API_DEF LY_ERR
lyht_find_with_val_cb(const struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb val_equal, void **match_p)
{
    struct ly_ht_rec *rec;

    lyht_find_rec(ht, val_p, hash, 0, val_equal ? val_equal : ht->val_equal, NULL, NULL, &rec);

    if (rec && match_p) {
        *match_p = rec->val;
    }
    return rec ? LY_SUCCESS : LY_ENOTFOUND;
}

LIBYANG_API_DEF LY_ERR
lyht_find_next_with_collision_cb(const struct ly_ht *ht, void *val_p, uint32_t hash,
        lyht_value_equal_cb collision_val_equal, void **match_p)
{
    struct ly_ht_rec *rec, *crec;
    uint32_t rec_idx;
    uint32_t i;

    /* find the record of the previously found value */
    if (lyht_find_rec(ht, val_p, hash, 1, ht->val_equal, &crec, &i, &rec)) {
        /* not found, cannot happen */
        LOGINT_RET(NULL);
    }

    for (rec_idx = rec->next, rec = lyht_get_rec(ht->recs, ht->rec_size, rec_idx);
            rec_idx != LYHT_NO_RECORD;
            rec_idx = rec->next, rec = lyht_get_rec(ht->recs, ht->rec_size, rec_idx)) {

        if (rec->hash != hash) {
            continue;
        }

        if (collision_val_equal) {
            if (collision_val_equal(val_p, &rec->val, 0, ht->cb_data)) {
                /* even the value matches */
                if (match_p) {
                    *match_p = rec->val;
                }
                return LY_SUCCESS;
            }
        } else if (ht->val_equal(val_p, &rec->val, 0, ht->cb_data)) {
            /* even the value matches */
            if (match_p) {
                *match_p = rec->val;
            }
            return LY_SUCCESS;
        }
    }

    /* the last equal value was already returned */
    return LY_ENOTFOUND;
}

LIBYANG_API_DEF LY_ERR
lyht_find_next(const struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p)
{
    return lyht_find_next_with_collision_cb(ht, val_p, hash, NULL, match_p);
}

static LY_ERR
_lyht_insert_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal,
        void **match_p, int check)
{
    uint32_t hlist_idx = hash & (ht->size - 1);
    LY_ERR r, ret = LY_SUCCESS;
    struct ly_ht_rec *rec, *prev_rec;
    lyht_value_equal_cb old_val_equal = NULL;
    uint32_t rec_idx;

    if (check) {
        if (lyht_find_rec(ht, val_p, hash, 1, ht->val_equal, NULL, NULL, &rec) == LY_SUCCESS) {
            if (rec && match_p) {
                *match_p = rec->val;
            }
            return LY_EEXIST;
        }
    }

    rec_idx = ht->first_free_rec;
    assert(rec_idx < ht->size);
    rec = lyht_get_rec(ht->recs, ht->rec_size, rec_idx);
    ht->first_free_rec = rec->next;

    if (ht->hlists[hlist_idx].first == LYHT_NO_RECORD) {
        ht->hlists[hlist_idx].first = rec_idx;
    } else {
        prev_rec = lyht_get_rec(ht->recs, ht->rec_size, ht->hlists[hlist_idx].last);
        prev_rec->next = rec_idx;
    }
    rec->next = LYHT_NO_RECORD;
    ht->hlists[hlist_idx].last = rec_idx;

    rec->hash = hash;
    memcpy(&rec->val, val_p, ht->rec_size - SIZEOF_LY_HT_REC);
    if (match_p) {
        *match_p = (void *)&rec->val;
    }

    /* check size & enlarge if needed */
    ++ht->used;
    if (ht->resize) {
        r = (ht->used * LYHT_HUNDRED_PERCENTAGE) / ht->size;
        if ((ht->resize == 1) && (r >= LYHT_FIRST_SHRINK_PERCENTAGE)) {
            /* enable shrinking */
            ht->resize = 2;
        }
        if ((ht->resize == 2) && (r >= LYHT_ENLARGE_PERCENTAGE)) {
            if (resize_val_equal) {
                old_val_equal = lyht_set_cb(ht, resize_val_equal);
            }

            /* enlarge */
            ret = lyht_resize(ht, 1, check);
            /* if hash_table was resized, we need to find new matching value */
            if ((ret == LY_SUCCESS) && match_p) {
                ret = lyht_find(ht, val_p, hash, match_p);
                assert(!ret);
            }

            if (resize_val_equal) {
                lyht_set_cb(ht, old_val_equal);
            }
        }
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyht_insert_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal,
        void **match_p)
{
    return _lyht_insert_with_resize_cb(ht, val_p, hash, resize_val_equal, match_p, 1);
}

LIBYANG_API_DEF LY_ERR
lyht_insert(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p)
{
    return _lyht_insert_with_resize_cb(ht, val_p, hash, NULL, match_p, 1);
}

LIBYANG_API_DEF LY_ERR
lyht_insert_no_check(struct ly_ht *ht, void *val_p, uint32_t hash, void **match_p)
{
    return _lyht_insert_with_resize_cb(ht, val_p, hash, NULL, match_p, 0);
}

LIBYANG_API_DEF LY_ERR
lyht_remove_with_resize_cb(struct ly_ht *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal)
{
    struct ly_ht_rec *found_rec, *prev_rec, *rec;
    uint32_t hlist_idx = hash & (ht->size - 1);
    LY_ERR r, ret = LY_SUCCESS;
    lyht_value_equal_cb old_val_equal = NULL;
    uint32_t prev_rec_idx;
    uint32_t rec_idx;

    if (lyht_find_rec(ht, val_p, hash, 1, ht->val_equal, NULL, NULL, &found_rec)) {
        LOGARG(NULL, hash);
        return LY_ENOTFOUND;
    }

    prev_rec_idx = LYHT_NO_RECORD;
    LYHT_ITER_HLIST_RECS(ht, hlist_idx, rec_idx, rec) {
        if (rec == found_rec) {
            break;
        }
        prev_rec_idx = rec_idx;
    }

    if (prev_rec_idx == LYHT_NO_RECORD) {
        ht->hlists[hlist_idx].first = rec->next;
        if (rec->next == LYHT_NO_RECORD) {
            ht->hlists[hlist_idx].last = LYHT_NO_RECORD;
        }
    } else {
        prev_rec = lyht_get_rec(ht->recs, ht->rec_size, prev_rec_idx);
        prev_rec->next = rec->next;
        if (rec->next == LYHT_NO_RECORD) {
            ht->hlists[hlist_idx].last = prev_rec_idx;
        }
    }

    rec->next = ht->first_free_rec;
    ht->first_free_rec = rec_idx;

    /* check size & shrink if needed */
    --ht->used;
    if (ht->resize == 2) {
        r = (ht->used * LYHT_HUNDRED_PERCENTAGE) / ht->size;
        if ((r < LYHT_SHRINK_PERCENTAGE) && (ht->size > LYHT_MIN_SIZE)) {
            if (resize_val_equal) {
                old_val_equal = lyht_set_cb(ht, resize_val_equal);
            }

            /* shrink */
            ret = lyht_resize(ht, -1, 1);

            if (resize_val_equal) {
                lyht_set_cb(ht, old_val_equal);
            }
        }
    }

    return ret;
}

LIBYANG_API_DEF LY_ERR
lyht_remove(struct ly_ht *ht, void *val_p, uint32_t hash)
{
    return lyht_remove_with_resize_cb(ht, val_p, hash, NULL);
}

LIBYANG_API_DEF uint32_t
lyht_get_fixed_size(uint32_t item_count)
{
    if (item_count == 0) {
        return 1;
    }

    /* return next power of 2 (greater or equal) */
    item_count--;
    item_count |= item_count >> 1;
    item_count |= item_count >> 2;
    item_count |= item_count >> 4;
    item_count |= item_count >> 8;
    item_count |= item_count >> 16;

    return item_count + 1;
}
