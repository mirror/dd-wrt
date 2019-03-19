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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "common.h"
#include "context.h"
#include "hash_table.h"

static int
lydict_val_eq(void *val1_p, void *val2_p, int UNUSED(mod), void *cb_data)
{
    if (!val1_p || !val2_p) {
        LOGARG;
        return 0;
    }

    const char *str1 = ((struct dict_rec *)val1_p)->value;
    const char *str2 = ((struct dict_rec *)val2_p)->value;

    if (!str1 || !str2 || !cb_data) {
        LOGARG;
        return 0;
    }

    if (strncmp(str1, str2, *(size_t *)cb_data) == 0) {
        return 1;
    }

    return 0;
}

void
lydict_init(struct dict_table *dict)
{
    if (!dict) {
        LOGARG;
        return;
    }

    dict->hash_tab = lyht_new(1024, sizeof(struct dict_rec), lydict_val_eq, NULL, 1);
    LY_CHECK_ERR_RETURN(!dict->hash_tab, LOGINT(NULL), );
    pthread_mutex_init(&dict->lock, NULL);
}

void
lydict_clean(struct dict_table *dict)
{
    unsigned int i;
    struct dict_rec *dict_rec  = NULL;
    struct ht_rec *rec = NULL;

    if (!dict) {
        LOGARG;
        return;
    }

    for (i = 0; i < dict->hash_tab->size; i++) {
        /* get ith record */
        rec = (struct ht_rec *)&dict->hash_tab->recs[i * dict->hash_tab->rec_size];
        if (rec->hits == 1) {
            /*
             * this should not happen, all records inserted into
             * dictionary are supposed to be removed using lydict_remove()
             * before calling lydict_clean()
             */
            dict_rec  = (struct dict_rec *)rec->val;
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
 * Bob Jenkin's one-at-a-time hash
 * http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Spooky hash is faster, but it works only for little endian architectures.
 */
static uint32_t
dict_hash(const char *key, size_t len)
{
    uint32_t hash, i;

    for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
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

API void
lydict_remove(struct ly_ctx *ctx, const char *value)
{
    size_t len;
    int ret;
    uint32_t hash;
    struct dict_rec rec, *match = NULL;
    char *val_p;

    if (!value || !ctx) {
        return;
    }

    len = strlen(value);
    hash = dict_hash(value, len);

    /* create record for lyht_find call */
    rec.value = (char *)value;
    rec.refcount = 0;

    pthread_mutex_lock(&ctx->dict.lock);
    /* set len as data for compare callback */
    lyht_set_cb_data(ctx->dict.hash_tab, (void *)&len);
    /* check if value is already inserted */
    ret = lyht_find(ctx->dict.hash_tab, &rec, hash, (void **)&match);

    if (ret == 0) {
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
            ret = lyht_remove(ctx->dict.hash_tab, &rec, hash);
            free(val_p);
            LY_CHECK_ERR_GOTO(ret, LOGINT(ctx), finish);
        }
    }

finish:
    pthread_mutex_unlock(&ctx->dict.lock);
}

static char *
dict_insert(struct ly_ctx *ctx, char *value, size_t len, int zerocopy)
{
    struct dict_rec *match = NULL, rec;
    int ret = 0;
    uint32_t hash;

    hash = dict_hash(value, len);
    /* set len as data for compare callback */
    lyht_set_cb_data(ctx->dict.hash_tab, (void *)&len);
    /* create record for lyht_insert */
    rec.value = value;
    rec.refcount = 1;

    LOGDBG(LY_LDGDICT, "inserting \"%s\"", rec.value);
    ret = lyht_insert(ctx->dict.hash_tab, (void *)&rec, hash, (void **)&match);
    if (ret == 1) {
        match->refcount++;
        if (zerocopy) {
            free(value);
        }
    } else if (ret == 0) {
        if (!zerocopy) {
            /*
             * allocate string for new record
             * record is already inserted in hash table
             */
            match->value = malloc(sizeof *match->value * (len + 1));
            LY_CHECK_ERR_RETURN(!match->value, LOGMEM(ctx), NULL);
            memcpy(match->value, value, len);
            match->value[len] = '\0';
        }
    } else {
        /* lyht_insert returned error */
        LOGINT(ctx);
        return NULL;
    }

    return match->value;
}

API const char *
lydict_insert(struct ly_ctx *ctx, const char *value, size_t len)
{
    const char *result;

    if (!value) {
        return NULL;
    }

    if (!len) {
        len = strlen(value);
    }

    pthread_mutex_lock(&ctx->dict.lock);
    result = dict_insert(ctx, (char *)value, len, 0);
    pthread_mutex_unlock(&ctx->dict.lock);

    return result;
}

API const char *
lydict_insert_zc(struct ly_ctx *ctx, char *value)
{
    const char *result;

    if (!value) {
        return NULL;
    }

    pthread_mutex_lock(&ctx->dict.lock);
    result = dict_insert(ctx, value, strlen(value), 1);
    pthread_mutex_unlock(&ctx->dict.lock);

    return result;
}

struct ht_rec *
lyht_get_rec(unsigned char *recs, uint16_t rec_size, uint32_t idx)
{
    return (struct ht_rec *)&recs[idx * rec_size];
}

struct hash_table *
lyht_new(uint32_t size, uint16_t val_size, values_equal_cb val_equal, void *cb_data, int resize)
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
    LY_CHECK_ERR_RETURN(!ht, LOGMEM(NULL), NULL);

    ht->used = 0;
    ht->size = size;
    ht->val_equal = val_equal;
    ht->cb_data = cb_data;
    ht->resize = (uint16_t)resize;

    ht->rec_size = (sizeof(struct ht_rec) - 1) + val_size;
    /* allocate the records correctly */
    ht->recs = calloc(size, ht->rec_size);
    LY_CHECK_ERR_RETURN(!ht->recs, free(ht); LOGMEM(NULL), NULL);

    return ht;
}

values_equal_cb
lyht_set_cb(struct hash_table *ht, values_equal_cb new_val_equal)
{
    values_equal_cb prev;

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

    if (!orig) {
        return NULL;
    }

    ht = lyht_new(orig->size, orig->rec_size - (sizeof(struct ht_rec) - 1), orig->val_equal, orig->cb_data, orig->resize ? 1 : 0);
    if (!ht) {
        return NULL;
    }

    memcpy(ht->recs, orig->recs, orig->used * orig->rec_size);
    ht->used = orig->used;
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

static int
lyht_resize(struct hash_table *ht, int enlarge)
{
    struct ht_rec *rec;
    unsigned char *old_recs;
    uint32_t i, old_size;
    int ret;

    old_recs = ht->recs;
    old_size = ht->size;

    if (enlarge) {
        /* double the size */
        ht->size <<= 1;
    } else {
        /* half the size */
        ht->size >>= 1;
    }

    ht->recs = calloc(ht->size, ht->rec_size);
    LY_CHECK_ERR_RETURN(!ht->recs, LOGMEM(NULL); ht->recs = old_recs; ht->size = old_size, -1);

    /* reset used, it will increase again */
    ht->used = 0;

    /* add all the old records into the new records array */
    for (i = 0; i < old_size; ++i) {
        rec = lyht_get_rec(old_recs, ht->rec_size, i);
        if (rec->hits > 0) {
            ret = lyht_insert(ht, rec->val, rec->hash, NULL);
            assert(!ret);
            (void)ret;
        }
    }

    /* final touches */
    free(old_recs);
    return 0;
}

/* return: 0 - hash found, returned its record,
 *         1 - hash not found, returned the record where it would be inserted */
static int
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
            return 1;
        }
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    }
    if (rec->hits == 0) {
        /* we could not find the value */
        if (rec_p && !*rec_p) {
            *rec_p = rec;
        }
        return 1;
    }

    /* we have found a record with equal (shortened) hash */
    if (rec_p) {
        *rec_p = rec;
    }
    return 0;
}

/**
 * @brief Search for the next collision.
 *
 * @param[in] ht Hash table to search in.
 * @param[in,out] last Last returned collision record.
 * @param[in] first First collision record (hits > 1).
 * @return 0 when hash collision found, \p last points to this next collision,
 *         1 when hash collision not found, \p last points to the record where it would be inserted.
 */
static int
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
            return 1;
        }

        if (((*last)->hits == -1) && !empty) {
            empty = *last;
        }
    } while (((*last)->hits != 0) && (((*last)->hits == -1) || (((*last)->hash & (ht->size - 1)) != idx)));

    if ((*last)->hits > 0) {
        /* we found a collision */
        assert((*last)->hits == 1);
        return 0;
    }

    /* no next collision found, return the record where it would be inserted */
    if (empty) {
        *last = empty;
    } /* else (*last)->hits == 0, it is already correct */
    return 1;
}

int
lyht_find(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    struct ht_rec *rec, *crec;
    uint32_t i, c;
    int r;

    if (lyht_find_first(ht, hash, &rec)) {
        /* not found */
        return 1;
    }
    if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 0, ht->cb_data)) {
        /* even the value matches */
        if (match_p) {
            *match_p = rec->val;
        }
        return 0;
    }

    /* some collisions, we need to go through them, too */
    crec = rec;
    c = rec->hits;
    for (i = 1; i < c; ++i) {
        r = lyht_find_collision(ht, &rec, crec);
        assert(!r);
        (void)r;

        /* compare values */
        if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 0, ht->cb_data)) {
            if (match_p) {
                *match_p = rec->val;
            }
            return 0;
        }
    }

    /* not found even in collisions */
    return 1;
}

int
lyht_find_next(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    struct ht_rec *rec, *crec;
    uint32_t i, c;
    int r, found = 0;

    if (lyht_find_first(ht, hash, &rec)) {
        /* not found, cannot happen */
        assert(0);
    }

    if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
        /* previously returned value */
        found = 1;
    }

    if (rec->hits == 1) {
        /* there are no more similar values */
        assert(rec->hash == hash);
        assert(found);
        return 1;
    }

    /* go through collisions and find next one after the previous one */
    crec = rec;
    c = rec->hits;
    for (i = 1; i < c; ++i) {
        r = lyht_find_collision(ht, &rec, crec);
        assert(!r);
        (void)r;

        if (rec->hash != hash) {
            /* a normal collision, we are not interested in those */
            continue;
        }

        if (found) {
            /* next value with equal hash, found our value */
            if (match_p) {
                *match_p = rec->val;
            }
            return 0;
        }

        if (!ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
            /* already returned value, skip */
            continue;
        }

        /* this one was returned previously, continue looking */
        found = 1;
    }

    /* the last equal value was already returned */
    assert(found);
    return 1;
}

int
lyht_insert_with_resize_cb(struct hash_table *ht, void *val_p, uint32_t hash,
                           values_equal_cb resize_val_equal, void **match_p)
{
    struct ht_rec *rec, *crec = NULL;
    int32_t i;
    int r, ret;
    values_equal_cb old_val_equal;

    if (!lyht_find_first(ht, hash, &rec)) {
        /* we found matching shortened hash */
        if ((rec->hash == hash) && ht->val_equal(val_p, &rec->val, 1, ht->cb_data)) {
            /* even the value matches */
            if (match_p) {
                *match_p = (void *)&rec->val;
            }
            return 1;
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
                return 1;
            }
        }

        /* value not found, get the record where it will be inserted */
        r = lyht_find_collision(ht, &rec, crec);
        assert(r);
    }

    /* insert it into the returned record */
    assert(rec->hits < 1);
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
    ret = 0;
    ++ht->used;
    if (ht->resize) {
        r = (ht->used * 100) / ht->size;
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
            if (ret == 0 && match_p) {
                lyht_find(ht, val_p, hash, match_p);
            }

            if (resize_val_equal) {
                lyht_set_cb(ht, old_val_equal);
            }
        }
    }
    return ret;
}

int
lyht_insert(struct hash_table *ht, void *val_p, uint32_t hash, void **match_p)
{
    return lyht_insert_with_resize_cb(ht, val_p, hash, NULL, match_p);
}

int
lyht_remove(struct hash_table *ht, void *val_p, uint32_t hash)
{
    struct ht_rec *rec, *crec;
    int32_t i;
    int first_matched = 0, r, ret;

    if (lyht_find_first(ht, hash, &rec)) {
        /* hash not found */
        return 1;
    }
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
        assert(!first_matched);
        return 1;
    }

    /* check size & shrink if needed */
    ret = 0;
    --ht->used;
    if (ht->resize == 2) {
        r = (ht->used * 100) / ht->size;
        if ((r < LYHT_SHRINK_PERCENTAGE) && (ht->size > LYHT_MIN_SIZE)) {
            /* shrink */
            ret = lyht_resize(ht, 0);
        }
    }

    return ret;
}
