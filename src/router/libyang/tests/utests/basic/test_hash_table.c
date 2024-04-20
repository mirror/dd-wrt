/*
 * @file test_hash_table.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from hash_table.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdlib.h>

#include "common.h"
#include "hash_table.h"

struct ht_rec *lyht_get_rec(unsigned char *recs, uint16_t rec_size, uint32_t idx);

static void
test_invalid_arguments(void **state)
{
    assert_int_equal(LY_EINVAL, lydict_insert(NULL, NULL, 0, NULL));
    CHECK_LOG("Invalid argument ctx (lydict_insert()).", NULL);

    assert_int_equal(LY_EINVAL, lydict_insert_zc(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument ctx (lydict_insert_zc()).", NULL);
    assert_int_equal(LY_EINVAL, lydict_insert_zc(UTEST_LYCTX, NULL, NULL));
    CHECK_LOG_CTX("Invalid argument str_p (lydict_insert_zc()).", NULL);
}

static void
test_dict_hit(void **state)
{
    const char *str1, *str2, *str3;

    /* insert 2 strings, one of them repeatedly */
    assert_int_equal(LY_SUCCESS, lydict_insert(UTEST_LYCTX, "test1", 0, &str1));
    assert_non_null(str1);
    /* via zerocopy we have to get the same pointer as provided */
    assert_non_null(str2 = strdup("test2"));
    assert_int_equal(LY_SUCCESS, lydict_insert_zc(UTEST_LYCTX, (char *)str2, &str3));
    assert_ptr_equal(str2, str3);
    /* here we get the same pointer as in case the string was inserted first time */
    assert_int_equal(LY_SUCCESS, lydict_insert(UTEST_LYCTX, "test1", 0, &str2));
    assert_non_null(str2);
    assert_ptr_equal(str1, str2);

    /* remove strings, but the repeatedly inserted only once */
    lydict_remove(UTEST_LYCTX, "test1");
    lydict_remove(UTEST_LYCTX, "test2");

    /* destroy dictionary - should raise warning about data presence */
    ly_ctx_destroy(UTEST_LYCTX);
    UTEST_LYCTX = NULL;
    CHECK_LOG("String \"test1\" not freed from the dictionary, refcount 1", NULL);

#ifndef NDEBUG
    /* cleanup */
    free((char *)str1);
#endif
}

static uint8_t
ht_equal_clb(void *val1, void *val2, uint8_t mod, void *cb_data)
{
    int *v1, *v2;

    (void)mod;
    (void)cb_data;

    v1 = (int *)val1;
    v2 = (int *)val2;

    return *v1 == *v2;
}

static void
test_ht_basic(void **state)
{
    uint32_t i;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 0));

    i = 2;
    assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_insert(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_ENOTFOUND, lyht_remove(ht, &i, i));
    CHECK_LOG("Invalid argument hash (lyht_remove_with_resize_cb()).", NULL);

    lyht_free(ht);
}

static void
test_ht_resize(void **state)
{
    uint32_t i;
    struct ht_rec *rec;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 1));
    assert_int_equal(8, ht->size);

    /* insert records into indexes 2-7 */
    for (i = 2; i < 8; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_insert(ht, &i, i, NULL));
    }
    /* check that table resized */
    assert_int_equal(16, ht->size);

    /* check expected content of the table */
    for (i = 0; i < 16; ++i) {
        if ((i >= 2) && (i < 8)) {
            /* inserted data on indexes 2-7 */
            rec = lyht_get_rec(ht->recs, ht->rec_size, i);
            assert_int_equal(1, rec->hits);
            assert_int_equal(i, rec->hash);
        } else {
            /* nothing otherwise */
            rec = lyht_get_rec(ht->recs, ht->rec_size, i);
            assert_int_equal(0, rec->hits);
        }
    }

    /* removing not present data should fail */
    for (i = 0; i < 2; ++i) {
        UTEST_LOG_CLEAN;
        assert_int_equal(LY_ENOTFOUND, lyht_remove(ht, &i, i));
        CHECK_LOG("Invalid argument hash (lyht_remove_with_resize_cb()).", NULL);
    }
    /* removing present data, resize should happened
     * when we are below 25% of the table filled, so with 3 records left */
    for ( ; i < 5; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    }
    assert_int_equal(8, ht->size);

    /* remove the rest */
    for ( ; i < 8; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    }

    for (i = 0; i < 8; ++i) {
        assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    }

    /* cleanup */
    lyht_free(ht);
}

static void
test_ht_collisions(void **UNUSED(state))
{
#define GET_REC_INT(rec) (*((uint32_t *)&(rec)->val))

    uint32_t i;
    struct ht_rec *rec;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 1));

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
    assert_int_equal(GET_REC_INT(rec), i);
    ++i;
    for ( ; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 1);
        assert_int_equal(GET_REC_INT(rec), i);
    }
    for ( ; i < 8; ++i) {
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
    assert_int_equal(GET_REC_INT(rec), 5);
    ++i;
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 1);
    assert_int_equal(GET_REC_INT(rec), 3);
    ++i;
    for ( ; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for ( ; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    for (i = 0; i < 3; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
    }
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_SUCCESS);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_SUCCESS);
    ++i;
    for ( ; i < 8; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
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
    for ( ; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for ( ; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    lyht_free(ht);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_invalid_arguments),
        UTEST(test_dict_hit),
        UTEST(test_ht_basic),
        UTEST(test_ht_resize),
        UTEST(test_ht_collisions),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
