/**
 * @file test_hash_table.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for hash table.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"
#include "hash_table.h"

static struct hash_table *ht;

static int
val_equal(void *val1, void *val2, int mod, void *cb_data)
{
    int *v1, *v2;
    (void)mod;
    (void)cb_data;

    v1 = (int *)val1;
    v2 = (int *)val2;

    return *v1 == *v2;
}

static int
setup_f(void **state)
{
    (void)state;

    ht = lyht_new(8, sizeof(int), val_equal, NULL, 0);
    if (!ht) {
        fprintf(stderr, "Failed to create hash table.\n");
        return -1;
    }

    return 0;
}

static int
setup_f_resize(void **state)
{
    (void)state;

    ht = lyht_new(8, sizeof(int), val_equal, NULL, 1);
    if (!ht) {
        fprintf(stderr, "Failed to create hash table.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void)state;

    lyht_free(ht);
    return 0;
}

static void
test_simple(void **state)
{
    int i;
    (void)state;

    i = 2;
    assert_int_equal(lyht_insert(ht, &i, i, NULL), 0);
    assert_int_equal(lyht_find(ht, &i, i, NULL), 0);
    assert_int_equal(lyht_remove(ht, &i, i), 0);
    assert_int_equal(lyht_find(ht, &i, i, NULL), 1);
    assert_int_equal(lyht_remove(ht, &i, i), 1);
}

static void
test_half_full(void **state)
{
    int i, j, k, l;
    (void)state;

    i = 0;
    assert_int_equal(lyht_insert(ht, &i, i, NULL), 0);
    j = 2;
    assert_int_equal(lyht_insert(ht, &j, j, NULL), 0);
    k = 4;
    assert_int_equal(lyht_insert(ht, &k, k, NULL), 0);
    l = 6;
    assert_int_equal(lyht_insert(ht, &l, l, NULL), 0);

    assert_int_equal(lyht_find(ht, &i, i, NULL), 0);
    assert_int_equal(lyht_find(ht, &j, j, NULL), 0);
    assert_int_equal(lyht_find(ht, &k, k, NULL), 0);
    assert_int_equal(lyht_find(ht, &l, l, NULL), 0);

    assert_int_equal(lyht_remove(ht, &j, j), 0);
    assert_int_equal(lyht_remove(ht, &k, k), 0);

    assert_int_equal(lyht_find(ht, &i, i, NULL), 0);
    assert_int_equal(lyht_find(ht, &j, j, NULL), 1);
    assert_int_equal(lyht_find(ht, &k, k, NULL), 1);
    assert_int_equal(lyht_find(ht, &l, l, NULL), 0);

    assert_int_equal(lyht_remove(ht, &i, i), 0);
    assert_int_equal(lyht_remove(ht, &l, l), 0);

    assert_int_equal(lyht_find(ht, &i, i, NULL), 1);
    assert_int_equal(lyht_find(ht, &j, j, NULL), 1);
    assert_int_equal(lyht_find(ht, &k, k, NULL), 1);
    assert_int_equal(lyht_find(ht, &l, l, NULL), 1);
}

static void
test_resize(void **state)
{
    int i;
    struct ht_rec *rec;
    (void)state;

    for (i = 2; i < 8; ++i) {
        assert_int_equal(lyht_insert(ht, &i, i, NULL), 0);
    }

    assert_int_equal(ht->size, 16);

    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 1);
        assert_int_equal(rec->hash, i);
    }
    for (; i < 16; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    for (i = 0; i < 2; ++i) {
        assert_int_equal(lyht_find(ht, &i, i, NULL), 1);
    }
    for (; i < 8; ++i) {
        assert_int_equal(lyht_find(ht, &i, i, NULL), 0);
    }

    for (i = 0; i < 2; ++i) {
        assert_int_equal(lyht_remove(ht, &i, i), 1);
    }
    for (; i < 8; ++i) {
        assert_int_equal(lyht_remove(ht, &i, i), 0);
    }

    assert_int_equal(ht->size, 8);

    for (i = 0; i < 8; ++i) {
        assert_int_equal(lyht_find(ht, &i, i, NULL), 1);
    }
}

#define GET_REC_VAL(rec) (*((int *)&(rec)->val))

static void
test_collisions(void **state)
{
    int i;
    struct ht_rec *rec;
    (void)state;

    for (i = 2; i < 6; ++i) {
        assert_int_equal(lyht_insert(ht, &i, 2, NULL), 0);
    }

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 4);
    assert_int_equal(GET_REC_VAL(rec), i);
    ++i;
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 1);
        assert_int_equal(GET_REC_VAL(rec), i);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    i = 4;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, -1);

    i = 2;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 2);
    assert_int_equal(GET_REC_VAL(rec), 5);
    ++i;
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 1);
    assert_int_equal(GET_REC_VAL(rec), 3);
    ++i;
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    for (i = 0; i < 3; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), 1);
    }
    assert_int_equal(lyht_find(ht, &i, 2, NULL), 0);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), 1);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), 0);
    ++i;
    for (; i < 8; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), 1);
    }

    i = 3;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);
    i = 5;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_simple, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_half_full, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_resize, setup_f_resize, teardown_f),
        cmocka_unit_test_setup_teardown(test_collisions, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
