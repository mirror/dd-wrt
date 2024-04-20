/**
 * @file hash_table.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary for storing strings and generic hash table
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
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

#include "common.h"
#include "compat.h"
#include "dict.h"
#include "log.h"

#define LYDICT_MIN_SIZE 1024

/**
 * @brief Comparison callback for dictionary's hash table
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lydict_val_eq(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *cb_data)
{
    LY_CHECK_ARG_RET(NULL, val1_p, val2_p, cb_data, 0);

    const char *str1 = ((struct dict_rec *)val1_p)->value;
    const char *str2 = ((struct dict_rec *)val2_p)->value;

    LY_CHECK_ERR_RET(!str1, LOGARG(NULL, val1_p), 0);
    LY_CHECK_ERR_RET(!str2, LOGARG(NULL, val2_p), 0);

    if (strncmp(str1, str2, *(size_t *)cb_data) == 0) {
        return 1;
    }

    return 0;
}

void
lydict_init(struct dict_table *dict)
{
    LY_CHECK_ARG_RET(NULL, dict, );

    dict->hash_tab = lyht_new(LYDICT_MIN_SIZE, sizeof(struct dict_rec), lydict_val_eq, NULL, 1);
    LY_CHECK_ERR_RET(!dict->hash_tab, LOGINT(NULL), );
    pthread_mutex_init(&dict->lock, NULL);
}

void
lydict_clean(struct dict_table *dict)
{
    struct dict_rec *dict_rec = NULL;
    struct ht_rec *rec = NULL;

    LY_CHECK_ARG_RET(NULL, dict, );

    for (uint32_t i = 0; i < dict->hash_tab->size; i++) {
        /* get ith record */
        rec = (struct ht_rec *)&dict->hash_tab->recs[i * dict->hash_tab->rec_size];
        if (rec->hits == 1) {
            /*
             * this should not happen, all records inserted into
             * dictionary are supposed to be removed using lydict_remove()
             * before calling lydict_clean()
             */
            dict_rec = (struct dict_rec *)rec->val;
            LOGWRN(NULL, "String \"%s\" not freed from the dictionary, refcount %d", dict_rec->value, dict_rec->refcount);
            /* if record wasn't removed before free string allocated for that record */
#ifdef NDEBUG
            free(dict_rec->value);
#endif
        }
    }

    /* free table and destroy mutex */
    lyht_free(dict->hash_tab);
    pthread_mutex_destroy(&dict->lock);
}

/*
 * Usage:
 * - init hash to 0
 * - repeatedly call dict_hash_multi(), provide hash from the last call
 * - call dict_hash_multi() with key_part = NULL to finish the hash
 */
uint32_t
dict_hash_multi(uint32_t hash, const char *key_part, size_t len)
{
    uint32_t i;

    if (key_part) {
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

/*
 * Bob Jenkin's one-at-a-time hash
 * http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Spooky hash is faster, but it works only for little endian architectures.
 */
uint32_t
dict_hash(const char *key, size_t len)
{
    uint32_t hash;

    hash = dict_hash_multi(0, key, len);
    return dict_hash_multi(hash, NULL, len);
}

static ly_bool
lydict_resize_val_eq(void *val1_p, void *val2_p, ly_bool mod, void *cb_data)
{
    LY_CHECK_ARG_RET(NULL, val1_p, val2_p, 0);

    const char *str1 = ((struct dict_rec *)val1_p)->value;
    const char *str2 = ((struct dict_rec *)val2_p)->value;

    LY_CHECK_ERR_RET(!str1, LOGARG(NULL, val1_p), 0);
    LY_CHECK_ERR_RET(!str2, LOGARG(NULL, val2_p), 0);

    if (mod) {
        /* used when inserting new values */
        if (strcmp(str1, str2) == 0) {
            return 1;
        }
    } else {
        /* used when finding the original value again in the resized table */
        return lydict_val_eq(val1_p, val2_p, mod, cb_data);
    }

    return 0;
}

API LY_ERR
lydict_remove(const struct ly_ctx *ctx, const char *value)
{
    LY_ERR ret = LY_SUCCESS;
    size_t len;
    uint32_t hash;
    struct dict_rec rec, *match = NULL;
    char *val_p;

    if (!ctx || !value) {
        return LY_SUCCESS;
    }

    LOGDBG(LY_LDGDICT, "removing \"%s\"", value);

    len = strlen(value);
    hash = dict_hash(value, len);

    /* create record for lyht_find call */
    rec.value = (char *)value;
    rec.refcount = 0;

    pthread_mutex_lock((pthread_mutex_t *)&ctx->dict.lock);
    /* set len as data for compare callback */
    lyht_set_cb_data(ctx->dict.hash_tab, (void *)&len);
    /* check if value is already inserted */
    ret = lyht_find(ctx->dict.hash_tab, &rec, hash, (void **)&match);

    if (ret == LY_SUCCESS) {
        LY_CHECK_ERR_GOTO(!match, LOGINT(ctx), finish);

        /* if value is already in dictionary, decrement reference counter */
        match->refcount--;
        if (match->refcount == 0) {
            /*
             * remove record
             * save pointer to stored string before lyht_remove to
             * free it after it is removed from hash table
             */
            val_p = match->value;
            ret = lyht_remove_with_resize_cb(ctx->dict.hash_tab, &rec, hash, lydict_resize_val_eq);
            free(val_p);
            LY_CHECK_ERR_GOTO(ret, LOGINT(ctx), finish);
        }
    } else if (ret == LY_ENOTFOUND) {
        LOGERR(ctx, LY_ENOTFOUND, "Value \"%s\" was not found in the dictionary.", value);
    } else {
        LOGINT(ctx);
    }

finish:
    pthread_mutex_unlock((pthread_mutex_t *)&ctx->dict.lock);
    return ret;
}

LY_ERR
dict_insert(const struct ly_ctx *ctx, char *value, size_t len, ly_bool zerocopy, const char **str_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct dict_rec *match = NULL, rec;
    uint32_t hash;

    LOGDBG(LY_LDGDICT, "inserting \"%.*s\"", (int)len, value);

    hash = dict_hash(value, len);
    /* set len as data for compare callback */
    lyht_set_cb_data(ctx->dict.hash_tab, (void *)&len);
    /* create record for lyht_insert */
    rec.value = value;
    rec.refcount = 1;

    ret = lyht_insert_with_resize_cb(ctx->dict.hash_tab, (void *)&rec, hash, lydict_resize_val_eq, (void **)&match);
    if (ret == LY_EEXIST) {
        match->refcount++;
        if (zerocopy) {
            free(value);
        }
        ret = LY_SUCCESS;
    } else if (ret == LY_SUCCESS) {
        if (!zerocopy) {
            /*
             * allocate string for new record
             * record is already inserted in hash table
             */
            match->value = malloc(sizeof *match->value * (len + 1));
            LY_CHECK_ERR_RET(!match->value, LOGMEM(ctx), LY_EMEM);
            memcpy(match->value, value, len);
            match->value[len] = '\0';
        }
    } else {
        /* lyht_insert returned error */
        if (zerocopy) {
            free(value);
        }
        return ret;
    }

    if (str_p) {
        *str_p = match->value;
    }

    return ret;
}

API LY_ERR
lydict_insert(const struct ly_ctx *ctx, const char *value, size_t len, const char **str_p)
{
    LY_ERR result;

    LY_CHECK_ARG_RET(ctx, ctx, str_p, LY_EINVAL);

    if (!value) {
        *str_p = NULL;
        return LY_SUCCESS;
    }

    if (!len) {
        len = strlen(value);
    }

    pthread_mutex_lock((pthread_mutex_t *)&ctx->dict.lock);
    result = dict_insert(ctx, (char *)value, len, 0, str_p);
    pthread_mutex_unlock((pthread_mutex_t *)&ctx->dict.lock);

    return result;
}

API LY_ERR
lydict_insert_zc(const struct ly_ctx *ctx, char *value, const char **str_p)
{
    LY_ERR result;

    LY_CHECK_ARG_RET(ctx, ctx, str_p, LY_EINVAL);

    if (!value) {
        *str_p = NULL;
        return LY_SUCCESS;
    }

    pthread_mutex_lock((pthread_mutex_t *)&ctx->dict.lock);
    result = dict_insert(ctx, value, strlen(value), 1, str_p);
    pthread_mutex_unlock((pthread_mutex_t *)&ctx->dict.lock);

    return result;
}

struct ht_rec *
lyht_get_rec(unsigned char *recs, uint16_t rec_size, uint32_t idx)
{
    return (struct ht_rec *)&recs[idx * rec_size];
}

struct hash_table *
lyht_new(uint32_t size, uint16_t val_size, lyht_value_equal_cb val_equal, void *cb_data, uint16_t resize)
{
    struct hash_table *ht;

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
    ht->invalid = 0;
    ht->val_equal = val_equal;
    ht->cb_data = cb_data;
    ht->resize = resize;

    ht->rec_size = (sizeof(struct ht_rec) - 1) + val_size;
    /* allocate the records correctly */
    ht->recs = calloc(size, ht->rec_size);
    LY_CHECK_ERR_RET(!ht->recs, free(ht); LOGMEM(NULL), NULL);

    return ht;
}

lyht_value_equal_cb
lyht_set_cb(struct hash_table *ht, lyht_value_equal_cb new_val_equal)
{
    lyht_value_equal_cb prev;

    prev = ht->val_equal;
    ht->val_equal = new_val_equal;
    return prev;
}

void *
lyht_set_cb_data(struct hash_table *ht, void *new_cb_data)
{
    void *prev;

    prev = ht->cb_data;
    ht->cb_data = new_cb_data;
    return prev;
}

struct hash_table *
lyht_dup(const struct hash_table *orig)
{
    struct hash_table *ht;

    LY_CHECK_ARG_RET(NULL, orig, NULL);

    ht = lyht_new(orig->size, orig->rec_size - (sizeof(struct ht_rec) - 1), orig->val_equal, orig->cb_data, orig->resize ? 1 : 0);
    if (!ht) {
        return NULL;
    }

    memcpy(ht->recs, orig->recs, orig->used * orig->rec_size);
    ht->used = orig->used;
    ht->invalid = orig->invalid;
    return ht;
}

void
lyht_free(struct hash_table *ht)
{
    if (ht) {
        free(ht->recs);
        free(ht);
    }
}

/**
 * @brief Resize a hash table.
 *
 * @param[in] ht Hash table to resize.
 * @param[in] operation Operation to perform. 1 to enlarge, -1 to shrink, 0 to only rehash all records.
 * @return LY_ERR value.
 */
static LY_ERR
lyht_resize(struct hash_table *ht, int operation)
{
    struct ht_rec *rec;
    unsigned char *old_recs;
    uint32_t i, old_size;

    old_recs = ht->recs;
    old_size = ht->size;

    if (operation > 0) {
        /* double the size */
        ht->size <<= 1;
    } else if (operation < 0) {
        /* half the size */
        ht->size >>= 1;
    }

    ht->recs = calloc(ht->size, ht->rec_size);
    LY_CHECK_ERR_RET(!ht->recs, LOGMEM(NULL); ht->recs = old_recs; ht->size = old_size, LY_EMEM);

    /* reset used and invalid, it will increase again */
    ht->used = 0;
    ht->invalid = 0;

    /* add all the old records into the new records array */
    for (i = 0; i < old_size; ++i) {
        rec = lyht_get_rec(old_recs, ht->rec_size, i);
        if (rec->hits > 0) {
            LY_ERR ret = lyht_insert(ht, rec->val, rec->hash, NULL);
            assert(!ret);
            (void)ret;
        }
    }

    /* final touches */
    free(old_recs);
    return LY_SUCCESS;
}

/**
 * @brief Search for the first match.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] hash Hash to find.
 * @param[out] rec_p Optional found record.
 * @return LY_SUCCESS hash found, returned its record,
 * @return LY_ENOTFOUND hash not found, returned the record where it would be inserted.
 */
static LY_ERR
lyht_find_first(struct hash_table *ht, uint32_t hash, struct ht_rec **rec_p)
{
    struct ht_rec *rec;
    uint32_t i, idx;

    if (rec_p) {
        *rec_p = NULL;
    }

    idx = i = hash & (ht->size - 1);
    rec = lyht_get_rec(ht->recs, ht->rec_size, idx);

    /* skip through overflow and deleted records */
    while ((rec->hits != 0) && ((rec->hits == -1) || ((rec->hash & (ht->size - 1)) != idx))) {
        if ((rec->hits == -1) && rec_p && !(*rec_p)) {
            /* remember this record for return */
            *rec_p = rec;
        }
        i = (i + 1) % ht->size;
        if (i == idx) {
            /* we went through all the records (very unlikely, but possible when many records are invalid),
             * just return not found */
            assert(!rec_p || *rec_p);
            return LY_ENOTFOUND;
        }
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    }
    if (rec->hits == 0) {
        /* we could not find the value */
        if (rec_p && !*rec_p) {
            *rec_p = rec;
        }
        return LY_ENOTFOUND;
    }

    /* we have found a record with equal (shortened) hash */
    if (rec_p) {
        *rec_p = rec;
    }
    return LY_SUCCESS;
}

/**
 * @brief Search for the next collision.
 *
 * @param[in] ht Hash table to search in.
 * @param[in,out] last Last returned collision record.
 * @param[in] first First collision record (hits > 1).
 * @return LY_SUCCESS when hash collision found, \p last points to this next collision,
 * @return LY_ENOTFOUND when hash collision not found, \p last points to the record where it would be inserted.
 */
static LY_ERR
lyht_find_collision(struct hash_table *ht, struct ht_rec **last, struct ht_rec *first)
{
    struct ht_rec *empty = NULL;
    uint32_t i, idx;

    assert(last && *last);

    idx = (*last)->hash & (ht->size - 1);
    i = (((unsigned char *)*last) - ht->recs) / ht->rec_size;

    do {
        i = (i + 1) % ht->size;
        *last = lyht_get_rec(ht->recs, ht->rec_size, i);
        if (*last == first) {
            /* we went through all the records (very unlikely, but possible when many records are invalid),
             * just return an invalid record */
            assert(empty);
            *last = empty;
            return LY_ENOTFOUND;
        }

        if (((*last)->hits == -1) && !empty) {
            empty = *last;
        }
    } while (((*last)->hits != 0) && (((*last)->hits == -1) || (((*last)->hash & (ht->size - 1)) != idx)));

    if ((*last)->hits > 0) {
        /* we found a collision */
        assert((*last)->hits == 1);
        return LY_SUCCESS;
    }

    /* no next collision found, return the record where it would be inserted */
    if (empty) {
        *last = empty;
    } /* else (*last)->hits == 0, it is already correct */
    return LY_ENOTFOUND;
}

/**
 * @brief Search for a record with specific value and hash.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] val_p Pointer to the value to find.
 * @param[in] hash Hash to find.
 * @param[in] mod Whether the operation modifies the hash table (insert or remove) or not (find).
 * @param[out] crec_p Optional found first record.
 * @param[out] col Optional collision number of @p rec_p, 0 for no collision.
 * @param[out] rec_p Found exact matching record, may be a collision of @p crec_p.
 * @return LY_ENOTFOUND if no record found,
 * @return LY_SUCCESS if record was found.
 */
static LY_ERR
lyht_find_rec(struct hash_table *ht, void *val_p, uint32_t hash, ly_bool mod, struct ht_rec **crec_p, uint32_t *col,
        struct ht_rec **rec_p)
{
    struct ht_rec *rec, *crec;
    uint32_t i, c;
    LY_ERR r;

    if (crec_p) {
        *crec_p = NULL;
    }
    if (col) {
        *col = 0;
    }
    *rec_p = NULL;

    if (lyht_find_first(ht, hash, &rec)) {
        /* not found */
        return LY_ENOTFOUND;
    }
    if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, mod, ht->cb_data)) {
        /* even the value matches */
        if (crec_p) {
            *crec_p = rec;
        }
        if (col) {
            *col = 0;
        }
        *rec_p = rec;
        return LY_SUCCESS;
    }

    /* some collisions, we need to go through them, too */
    crec = rec;
    c = crec->hits;
    for (i = 1; i < c; ++i) {
        r = lyht_find_collision(ht, &rec, crec);
        assert(!r);
        (void)r;

        /* compare values */
        if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, mod, ht->cb_data)) {
            if (crec_p) {
                *crec_p = crec;
            }
            if (col) {
                *col = i;
            }
            *rec_p = rec;
            return LY_SUCCESS;
        }
    }

    /* not found even in collisions */
    return LY_ENOTFOUND;
}

LY_ERR
lyht_find(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    struct ht_rec *rec;

    lyht_find_rec(ht, val_p, hash, 0, NULL, NULL, &rec);

    if (rec && match_p) {
        *match_p = rec->val;
    }
    return rec ? LY_SUCCESS : LY_ENOTFOUND;
}

LY_ERR
lyht_find_next(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    struct ht_rec *rec, *crec;
    uint32_t i, c;
    LY_ERR r;

    /* found the record of the previously found value */
    if (lyht_find_rec(ht, val_p, hash, 1, &crec, &i, &rec)) {
        /* not found, cannot happen */
        LOGINT_RET(NULL);
    }

    /* go through collisions and find next one after the previous one */
    c = crec->hits;
    for (++i; i < c; ++i) {
        r = lyht_find_collision(ht, &rec, crec);
        assert(!r);
        (void)r;

        if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 0, ht->cb_data)) {
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

LY_ERR
lyht_insert_with_resize_cb(struct hash_table *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal,
        void **match_p)
{
    LY_ERR r, ret = LY_SUCCESS;
    struct ht_rec *rec, *crec = NULL;
    int32_t i;
    lyht_value_equal_cb old_val_equal = NULL;

    if (!lyht_find_first(ht, hash, &rec)) {
        /* we found matching shortened hash */
        if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
            /* even the value matches */
            if (match_p) {
                *match_p = (void *)&rec->val;
            }
            return LY_EEXIST;
        }

        /* some collisions, we need to go through them, too */
        crec = rec;
        for (i = 1; i < crec->hits; ++i) {
            r = lyht_find_collision(ht, &rec, crec);
            assert(!r);

            /* compare values */
            if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
                if (match_p) {
                    *match_p = (void *)&rec->val;
                }
                return LY_EEXIST;
            }
        }

        /* value not found, get the record where it will be inserted */
        r = lyht_find_collision(ht, &rec, crec);
        assert(r);
    }

    /* insert it into the returned record */
    assert(rec->hits < 1);
    if (rec->hits < 0) {
        --ht->invalid;
    }
    rec->hash = hash;
    rec->hits = 1;
    memcpy(&rec->val, val_p, ht->rec_size - (sizeof(struct ht_rec) - 1));
    if (match_p) {
        *match_p = (void *)&rec->val;
    }

    if (crec) {
        /* there was a collision, increase hits */
        if (crec->hits == INT32_MAX) {
            LOGINT(NULL);
        }
        ++crec->hits;
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
            ret = lyht_resize(ht, 1);
            /* if hash_table was resized, we need to find new matching value */
            if ((ret == LY_SUCCESS) && match_p) {
                lyht_find(ht, val_p, hash, match_p);
            }

            if (resize_val_equal) {
                lyht_set_cb(ht, old_val_equal);
            }
        }
    }
    return ret;
}

LY_ERR
lyht_insert(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    return lyht_insert_with_resize_cb(ht, val_p, hash, NULL, match_p);
}

LY_ERR
lyht_remove_with_resize_cb(struct hash_table *ht, void *val_p, uint32_t hash, lyht_value_equal_cb resize_val_equal)
{
    struct ht_rec *rec, *crec;
    int32_t i;
    ly_bool first_matched = 0;
    LY_ERR r, ret = LY_SUCCESS;
    lyht_value_equal_cb old_val_equal;

    LY_CHECK_ERR_RET(lyht_find_first(ht, hash, &rec), LOGARG(NULL, hash), LY_ENOTFOUND); /* hash not found */

    if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
        /* even the value matches */
        first_matched = 1;
    }

    /* we always need to go through collisions */
    crec = rec;
    for (i = 1; i < crec->hits; ++i) {
        r = lyht_find_collision(ht, &rec, crec);
        assert(!r);

        /* compare values */
        if (!first_matched && (rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
            break;
        }
    }

    if (i < crec->hits) {
        /* one of collisions matched, reduce collision count, remove the record */
        assert(!first_matched);
        --crec->hits;
        rec->hits = -1;
    } else if (first_matched) {
        /* the first record matches */
        if (crec != rec) {
            /* ... so put the last collision in its place */
            rec->hits = crec->hits - 1;
            memcpy(crec, rec, ht->rec_size);
        }
        rec->hits = -1;
    } else {
        /* value not found even in collisions */
        return LY_ENOTFOUND;
    }

    /* check size & shrink if needed */
    --ht->used;
    ++ht->invalid;
    if (ht->resize == 2) {
        r = (ht->used * LYHT_HUNDRED_PERCENTAGE) / ht->size;
        if ((r < LYHT_SHRINK_PERCENTAGE) && (ht->size > LYHT_MIN_SIZE)) {
            if (resize_val_equal) {
                old_val_equal = lyht_set_cb(ht, resize_val_equal);
            }

            /* shrink */
            ret = lyht_resize(ht, -1);

            if (resize_val_equal) {
                lyht_set_cb(ht, old_val_equal);
            }
        }
    }

    /* rehash all records if needed */
    r = ((ht->size - ht->used - ht->invalid) * 100) / ht->size;
    if (r < LYHT_REHASH_PERCENTAGE) {
        if (resize_val_equal) {
            old_val_equal = lyht_set_cb(ht, resize_val_equal);
        }

        /* rehash */
        ret = lyht_resize(ht, 0);

        if (resize_val_equal) {
            lyht_set_cb(ht, old_val_equal);
        }
    }

    return ret;
}

LY_ERR
lyht_remove(struct hash_table *ht, void *val_p, uint32_t hash)
{
    return lyht_remove_with_resize_cb(ht, val_p, hash, NULL);
}
