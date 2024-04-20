/**
 * @file test_set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from set.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _POSIX_C_SOURCE 200809L /* strdup */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdlib.h>
#include <string.h>

#include "set.h"

static void
test_basics(void **UNUSED(state))
{
    struct ly_set *set;
    char *str;
    unsigned int u;
    void *ptr;
    uint32_t index;

    /* creation - everything is empty */
    assert_int_equal(LY_SUCCESS, ly_set_new(&set));
    assert_non_null(set);
    assert_int_equal(0, set->count);
    assert_int_equal(0, set->size);
    assert_null(set->objs);

    /* add a testing object */
    str = strdup("test string");
    assert_non_null(str);

    assert_int_equal(LY_SUCCESS, ly_set_add(set, str, 0, NULL));
    assert_int_not_equal(0, set->size);
    assert_int_equal(1, set->count);
    assert_non_null(set->objs);
    assert_non_null(set->objs[0]);

    /* check the presence of the testing data */
    assert_int_equal(1, ly_set_contains(set, str, &index));
    assert_int_equal(0, index);
    assert_int_equal(0, ly_set_contains(set, str - 1, NULL));

    /* remove data, but keep the set */
    u = set->size;
    ptr = set->objs;
    ly_set_clean(set, free);
    assert_int_equal(0, set->count);
    assert_int_equal(u, set->size);
    assert_ptr_equal(ptr, set->objs);

    /* remove buffer, but keep the set object */
    ly_set_erase(set, NULL);
    assert_int_equal(0, set->count);
    assert_int_equal(0, set->size);
    assert_ptr_equal(NULL, set->objs);

    /* final cleanup */
    ly_set_free(set, NULL);
}

static void
test_inval(void **state)
{
    struct ly_set set;

    memset(&set, 0, sizeof set);

    ly_set_clean(NULL, NULL);
    CHECK_LOG(NULL, NULL);

    ly_set_erase(NULL, NULL);
    CHECK_LOG(NULL, NULL);

    ly_set_free(NULL, NULL);
    CHECK_LOG(NULL, NULL);

    assert_int_equal(LY_EINVAL, ly_set_dup(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument set (ly_set_dup()).", NULL);

    assert_int_equal(LY_EINVAL, ly_set_add(NULL, NULL, 0, NULL));
    CHECK_LOG("Invalid argument set (ly_set_add()).", NULL);

    assert_int_equal(LY_EINVAL, ly_set_merge(NULL, NULL, 0, NULL));
    CHECK_LOG("Invalid argument trg (ly_set_merge()).", NULL);
    assert_int_equal(LY_SUCCESS, ly_set_merge(&set, NULL, 0, NULL));

    assert_int_equal(LY_EINVAL, ly_set_rm_index(NULL, 0, NULL));
    CHECK_LOG("Invalid argument set (ly_set_rm_index()).", NULL);
    assert_int_equal(LY_EINVAL, ly_set_rm_index(&set, 1, NULL));
    CHECK_LOG("Invalid argument index (ly_set_rm_index()).", NULL);

    assert_int_equal(LY_EINVAL, ly_set_rm(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument set (ly_set_rm()).", NULL);
    assert_int_equal(LY_EINVAL, ly_set_rm(&set, NULL, NULL));
    CHECK_LOG("Invalid argument object (ly_set_rm()).", NULL);
    assert_int_equal(LY_EINVAL, ly_set_rm(&set, &set, NULL));
    CHECK_LOG("Invalid argument object (ly_set_rm()).", NULL);
}

static void
test_duplication(void **UNUSED(state))
{
    struct ly_set *orig, *new;
    char *str;
    uint32_t index;

    assert_int_equal(LY_SUCCESS, ly_set_new(&orig));
    assert_non_null(orig);

    /* add a testing object */
    str = strdup("test string");
    assert_non_null(str);
    assert_int_equal(LY_SUCCESS, ly_set_add(orig, str, 0, &index));
    assert_int_equal(0, index);

    /* duplicate the set - without duplicator, so the new set will point to the same string */
    assert_int_equal(LY_SUCCESS, ly_set_dup(orig, NULL, &new));
    assert_non_null(new);
    assert_ptr_not_equal(orig, new);
    assert_int_equal(orig->count, new->count);
    assert_ptr_equal(orig->objs[0], new->objs[0]);

    ly_set_free(new, NULL);

    /* duplicate the set - with duplicator, so the new set will point to a different buffer with the same content */
    assert_int_equal(LY_SUCCESS, ly_set_dup(orig, (void *(*)(const void *))strdup, &new));
    assert_non_null(new);
    assert_ptr_not_equal(orig, new);
    assert_int_equal(orig->count, new->count);
    assert_ptr_not_equal(orig->objs[0], new->objs[0]);
    assert_string_equal(orig->objs[0], new->objs[0]);

    /* cleanup */
    ly_set_free(new, free);
    ly_set_free(orig, free);
}

static void
test_add(void **UNUSED(state))
{
    uint32_t u, index;
    char *str = "test string";
    struct ly_set set;

    memset(&set, 0, sizeof set);

    /* add a testing object */
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str, 0, &index));
    assert_int_equal(0, index);

    /* test avoiding data duplicities */
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str, 0, &index));
    assert_int_equal(0, index);
    assert_int_equal(1, set.count);
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str, 1, &index));
    assert_int_equal(1, index);
    assert_int_equal(2, set.count);

    /* test array resizing */
    u = set.size;
    for (uint32_t expected_index = 2; expected_index <= u; ++expected_index) {
        assert_int_equal(LY_SUCCESS, ly_set_add(&set, str, 1, &index));
        assert_int_equal(expected_index, index);
    }
    assert_true(u != set.size);

    /* cleanup */
    ly_set_erase(&set, NULL);
}

static void
test_merge(void **UNUSED(state))
{
    char *str1, *str2;
    struct ly_set one, two;

    memset(&one, 0, sizeof one);
    memset(&two, 0, sizeof two);

    str1 = strdup("string1");
    str2 = strdup("string2");

    /* fill first set
     * - str1 is the same as in two, so it must not be freed! */
    assert_int_equal(LY_SUCCESS, ly_set_add(&one, str1, 0, NULL));

    /* fill second set */
    assert_int_equal(LY_SUCCESS, ly_set_add(&two, str1, 0, NULL));
    assert_int_equal(LY_SUCCESS, ly_set_add(&two, str2, 0, NULL));

    /* merge with checking duplicities - only one item is added into one;
     * also without duplicating data, so it must not be freed at the end */
    assert_int_equal(LY_SUCCESS, ly_set_merge(&one, &two, 0, NULL));
    assert_int_equal(2, one.count);
    assert_ptr_equal(one.objs[1], two.objs[1]);

    /* clean and re-fill one (now duplicating str1, to allow testing duplicator) */
    ly_set_clean(&one, NULL);
    assert_int_equal(LY_SUCCESS, ly_set_add(&one, strdup(str1), 0, NULL));

    /* merge without checking duplicities - two items are added into one;
     * here also with duplicator */
    assert_int_equal(LY_SUCCESS, ly_set_merge(&one, &two, 1, (void *(*)(const void *))strdup));
    assert_int_equal(3, one.count);
    assert_ptr_not_equal(one.objs[1], two.objs[0]);
    assert_string_equal(one.objs[1], two.objs[0]);

    /* cleanup */
    ly_set_erase(&one, free);
    ly_set_erase(&two, free);
}

static void
test_rm(void **UNUSED(state))
{
    char *str1, *str2, *str3;
    struct ly_set set;

    memset(&set, 0, sizeof set);

    /* fill the set */
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, "string1", 0, NULL));
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, strdup("string2"), 0, NULL));
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, "string3", 0, NULL));

    /* remove by index ... */
    /* ... in the middle ... */
    assert_int_equal(LY_SUCCESS, ly_set_rm_index(&set, 1, free));
    assert_int_equal(2, set.count);
    assert_string_not_equal("string2", set.objs[0]);
    assert_string_not_equal("string2", set.objs[1]);
    /* ... last .. */
    assert_int_equal(LY_SUCCESS, ly_set_rm_index(&set, 1, NULL));
    assert_int_equal(1, set.count);
    assert_string_not_equal("string3", set.objs[0]);
    /* ... first .. */
    assert_int_equal(LY_SUCCESS, ly_set_rm_index(&set, 0, NULL));
    assert_int_equal(0, set.count);

    /* fill the set */
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str1 = "string1", 0, NULL));
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str2 = "string2", 0, NULL));
    assert_int_equal(LY_SUCCESS, ly_set_add(&set, str3 = strdup("string3"), 0, NULL));

    /* remove by pointer ... */
    /* ... in the middle ... */
    assert_int_equal(LY_SUCCESS, ly_set_rm(&set, str2, NULL));
    assert_int_equal(2, set.count);
    assert_string_not_equal("string2", set.objs[0]);
    assert_string_not_equal("string2", set.objs[1]);
    /* ... last (with destructor) .. */
    assert_int_equal(LY_SUCCESS, ly_set_rm(&set, str3, free));
    assert_int_equal(1, set.count);
    assert_string_not_equal("string3", set.objs[0]);
    /* ... first .. */
    assert_int_equal(LY_SUCCESS, ly_set_rm(&set, str1, NULL));
    assert_int_equal(0, set.count);

    /* cleanup */
    ly_set_erase(&set, NULL);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_basics),
        UTEST(test_duplication),
        UTEST(test_add),
        UTEST(test_merge),
        UTEST(test_rm),
        UTEST(test_inval),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
