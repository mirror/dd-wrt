/**
 * @file test_keys.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for keys in data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
};
static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/keys.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }


    /* schema */
    if (!lys_parse_path(st->ctx, schemafile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_keys_correct(void **state)
{
    struct state *st = (*state);
    const char *data = "<l xmlns=\"urn:libyang:tests:keys\"><key1>1</key1><key2>2</key2><value>a</value></l>";

    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_keys_correct2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    int rc;

    st->dt = lyd_new(NULL, ly_ctx_get_module(st->ctx, "keys", NULL, 1), "l");
    assert_ptr_not_equal(st->dt, NULL);

    node = lyd_new_leaf(st->dt, NULL, "key1", "1");
    assert_ptr_not_equal(node, NULL);

    node = lyd_new_leaf(st->dt, NULL, "key2", "2");
    assert_ptr_not_equal(node, NULL);

    node = lyd_new_leaf(st->dt, NULL, "value", "a");
    assert_ptr_not_equal(node, NULL);

    rc = lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL);
    assert_int_equal(rc, 0);
}

static void
test_keys_missing(void **state)
{
    struct state *st = (*state);
    const char *data = "<l xmlns=\"urn:libyang:tests:keys\"><key2>2</key2><value>a</value></l>";

    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);

    data = "<l xmlns=\"urn:libyang:tests:keys\"><key1>1</key1><value>a</value></l>";
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
}

static void
test_keys_missing2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    int rc;

    st->dt = lyd_new(NULL, ly_ctx_get_module(st->ctx, "keys", NULL, 1), "l");
    assert_ptr_not_equal(st->dt, NULL);

    node = lyd_new_leaf(st->dt, NULL, "key1", "1");
    assert_ptr_not_equal(node, NULL);

    rc = lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL);
    assert_int_not_equal(rc, 0);

    lyd_free(node);

    node = lyd_new_leaf(st->dt, NULL, "key2", "2");
    assert_ptr_not_equal(node, NULL);

    rc = lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL);
    assert_int_not_equal(rc, 0);
}

static void
test_keys_inorder(void **state)
{
    struct state *st = (*state);
    const char *data, *correct = "<l xmlns=\"urn:libyang:tests:keys\"><key1>1</key1><key2>2</key2><value>a</value></l>";
    char *printed;

    /* invalid order */
    data = "<l xmlns=\"urn:libyang:tests:keys\"><key2>2</key2><key1>1</key1><value>a</value></l>";
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_equal(st->dt, NULL);

    /* invalid order */
    data = "<l xmlns=\"urn:libyang:tests:keys\"><key1>1</key1><value>a</value><key2>2</key2></l>";
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_equal(st->dt, NULL);

    /* invalid order, not a strict parsing */
    data = "<l xmlns=\"urn:libyang:tests:keys\"><key2>2</key2><key1>1</key1><value>a</value></l>";
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&printed, st->dt, LYD_XML, 0), 0);
    assert_string_equal(printed, correct);
    free(printed);
    lyd_free_withsiblings(st->dt);

    /* invalid order, not a strict parsing */
    data = "<l xmlns=\"urn:libyang:tests:keys\"><key1>1</key1><value>a</value><key2>2</key2></l>";
    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&printed, st->dt, LYD_XML, 0), 0);
    assert_string_equal(printed, correct);
    free(printed);
}

static void
test_keys_inorder2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;

    st->dt = lyd_new(NULL, ly_ctx_get_module(st->ctx, "keys", NULL, 1), "l");
    assert_ptr_not_equal(st->dt, NULL);

    /* libyang is able to put the keys into a correct order */
    node = lyd_new_leaf(st->dt, NULL, "key2", "2");
    assert_ptr_not_equal(node, NULL);
    node = lyd_new_leaf(st->dt, NULL, "key1", "1");
    assert_ptr_not_equal(node, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    lyd_free_withsiblings(st->dt->child);

    /* libyang is able to put the keys into a correct order */
    node = lyd_new_leaf(st->dt, NULL, "key2", "2");
    assert_ptr_not_equal(node, NULL);
    node = lyd_new_leaf(st->dt, NULL, "value", "a");
    assert_ptr_not_equal(node, NULL);
    node = lyd_new_leaf(st->dt, NULL, "key1", "1");
    assert_ptr_not_equal(node, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_keys_correct, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_keys_correct2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_keys_missing, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_keys_missing2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_keys_inorder, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_keys_inorder2, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
