/**
 * @file test_xpath_1.1.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for YANG 1.1 XPath expression evaluation.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
    struct ly_set *set;
};

static const char *data1 =
"<top xmlns=\"urn:xpath-1.1\" xmlns:xp=\"urn:xpath-1.1\">"
    "<str1>aabbcc</str1>"
    "<str2>aabb</str2>"
    "<lref>aabbcc</lref>"
    "<instid>/xp:top/xp:str2</instid>"
    "<identref>ident2</identref>"
    "<enum>two</enum>"
    "<bits>flag1 flag3</bits>"
"</top>"
;

static const char *data2 =
"<top xmlns=\"urn:xpath-1.1\">"
    "<identref>ident2</identref>"
    "<enum>ten</enum>"
"</top>"
;

static int
setup_f(void **state)
{
    struct state *st;
    const char *schema = "xpath-1.1";
    const char *schemadir = TESTS_DIR"/api/files/";
    const struct lys_module *mod;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(schemadir, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    mod = ly_ctx_load_module(st->ctx, schema, NULL);
    if (!mod) {
        fprintf(stderr, "Failed to load module \"%s\".\n", schema);
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_set_free(st->set);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_func_re_match(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[re-match(., 'a+b+c+')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);
}

static void
test_func_deref(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "deref(/xpath-1.1:top/instid) | deref(/xpath-1.1:top/lref)");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 2);

    assert_string_equal(st->set->set.d[0]->schema->name, "str1");
    assert_string_equal(st->set->set.d[1]->schema->name, "str2");
}

static void
test_func_derived_from1(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from(., 'ident1')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_derived_from2(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from(., 'ident2')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 0);
}

static void
test_func_derived_from3(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from(., 'ident1')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_derived_from4(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from(., 'ident2')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 0);
}

static void
test_func_derived_from_or_self1(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from-or-self(., 'ident1')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_derived_from_or_self2(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from-or-self(., 'ident2')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_derived_from_or_self3(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from-or-self(., 'ident1')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_derived_from_or_self4(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[derived-from-or-self(., 'ident2')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_enum_value1(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[enum-value(.) = 2]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_enum_value2(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[enum-value(.) = 10]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_bit_is_set1(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[bit-is-set(., 'flag3')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 1);
}

static void
test_func_bit_is_set2(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    st->set = lyd_find_path(st->dt, "/xpath-1.1:top/*[bit-is-set(., 'flag2')]");
    assert_ptr_not_equal(st->set, NULL);
    assert_int_equal(st->set->number, 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_func_re_match, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_deref, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from1, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from3, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from4, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from_or_self1, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from_or_self2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from_or_self3, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_derived_from_or_self4, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_enum_value1, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_enum_value2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_bit_is_set1, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_func_bit_is_set2, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
