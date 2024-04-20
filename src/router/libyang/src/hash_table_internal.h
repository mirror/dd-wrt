/**
 * @file hash_table_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang hash table internal header
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_HASH_TABLE_INTERNAL_H_
#define LY_HASH_TABLE_INTERNAL_H_

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include "compat.h"
#include "hash_table.h"

/** reference value for 100% */
#define LYHT_HUNDRED_PERCENTAGE 100

/** when the table is at least this much percent full, it is enlarged (double the size) */
#define LYHT_ENLARGE_PERCENTAGE 75

/** only once the table is this much percent full, enable shrinking */
#define LYHT_FIRST_SHRINK_PERCENTAGE 50

/** when the table is less than this much percent full, it is shrunk (half the size) */
#define LYHT_SHRINK_PERCENTAGE 25

/** never shrink beyond this size */
#define LYHT_MIN_SIZE 8

/**
 * @brief Generic hash table record.
 */
struct ly_ht_rec {
    uint32_t hash;        /* hash of the value */
    uint32_t next;        /* index of next collision */
    unsigned char val[1]; /* arbitrary-size value */
} _PACKED;

/* real size, without taking the val[1] in account */
#define SIZEOF_LY_HT_REC (sizeof(struct ly_ht_rec) - 1)

struct ly_ht_hlist {
    uint32_t first;
    uint32_t last;
};

/**
 * @brief (Very) generic hash table.
 *
 * This structure implements a hash table, providing fast accesses to stored
 * values from their hash.
 *
 * The hash table structure contains 2 pointers to tables that are allocated
 * at initialization:
 * - a table of records: each record is composed of a struct ly_ht_rec header,
 *   followed by the user value. The header contains the hash value and a
 *   next index that can be used to chain records.
 * - a table of list heads: each list head entry contains the index of the
 *   first record in the records table whose hash (modulo hash table size)
 *   is equal to the index of the list head entry. The other matching records
 *   are chained together.
 *
 * The unused records are chained in first_free_rec, which contains the index
 * of the first unused record entry in the records table.
 *
 * The LYHT_NO_RECORD magic value is used when an index points to nothing.
 */
struct ly_ht {
    uint32_t used;        /* number of values stored in the hash table (filled records) */
    uint32_t size;        /* always holds 2^x == size (is power of 2), actually number of records allocated */
    lyht_value_equal_cb val_equal; /* callback for testing value equivalence */
    void *cb_data;        /* user data callback arbitrary value */
    uint16_t resize;      /* 0 - resizing is disabled, *
                           * 1 - enlarging is enabled, *
                           * 2 - both shrinking and enlarging is enabled */
    uint16_t rec_size;    /* real size (in bytes) of one record for accessing recs array */
    uint32_t first_free_rec; /* index of the first free record */
    struct ly_ht_hlist *hlists; /* pointer to the hlists table */
    unsigned char *recs;  /* pointer to the hash table itself (array of struct ht_rec) */
};

/* index that points to nothing */
#define LYHT_NO_RECORD UINT32_MAX

/* get the record associated to */
static inline struct ly_ht_rec *
lyht_get_rec(unsigned char *recs, uint16_t rec_size, uint32_t idx)
{
    return (struct ly_ht_rec *)&recs[idx * rec_size];
}

/* Iterate all records in a hlist */
#define LYHT_ITER_HLIST_RECS(ht, hlist_idx, rec_idx, rec)               \
    for (rec_idx = ht->hlists[hlist_idx].first,                         \
             rec = lyht_get_rec(ht->recs, ht->rec_size, rec_idx);       \
         rec_idx != LYHT_NO_RECORD;                                     \
         rec_idx = rec->next,                                           \
             rec = lyht_get_rec(ht->recs, ht->rec_size, rec_idx))

/* Iterate all records in the hash table */
#define LYHT_ITER_ALL_RECS(ht, hlist_idx, rec_idx, rec)              \
    for (hlist_idx = 0; hlist_idx < ht->size; hlist_idx++)           \
        LYHT_ITER_HLIST_RECS(ht, hlist_idx, rec_idx, rec)

/**
 * @brief Dictionary hash table record.
 */
struct ly_dict_rec {
    char *value;        /**< stored string */
    uint32_t refcount;  /**< reference count of the string */
};

/**
 * @brief Dictionary for storing repeated strings.
 */
struct ly_dict {
    struct ly_ht *hash_tab;
    pthread_mutex_t lock;
};

/**
 * @brief Initiate content (non-zero values) of the dictionary
 *
 * @param[in] dict Dictionary table to initiate
 */
void lydict_init(struct ly_dict *dict);

/**
 * @brief Cleanup the dictionary content
 *
 * @param[in] dict Dictionary table to cleanup
 */
void lydict_clean(struct ly_dict *dict);

#endif /* LY_HASH_TABLE_INTERNAL_H_ */
