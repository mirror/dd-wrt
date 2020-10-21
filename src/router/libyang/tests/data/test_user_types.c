/**
 * @file test_user_types.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for libyang internal user types.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
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
    const struct lys_module *mod;
    struct lyd_node *dt;
};

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/data/files", 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    st->mod = ly_ctx_load_module(st->ctx, "user-types", NULL);
    if (!st->mod) {
        fprintf(stderr, "Failed to load schema.\n");
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
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_yang_types(void **state)
{
    struct state *st = (struct state *)*state;

    /* date-and-time */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "2005-05-25T23:15:15.88888Z");
    assert_non_null(st->dt);
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "2005-05-31T23:15:15-08:59");
    assert_non_null(st->dt);
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "2005-05-31T23:15:15-23:00");
    assert_non_null(st->dt);
    lyd_free_withsiblings(st->dt);

    /* test 1 second before epoch (mktime returns -1, but it is a correct value), with and without DST */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "1970-01-01T00:59:59Z");
    assert_non_null(st->dt);
    lyd_free_withsiblings(st->dt);
    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "1969-12-31T23:59:59Z");
    assert_non_null(st->dt);
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "2005-05-31T23:15:15.-08:00");
    assert_null(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "yang1", "2005-02-29T23:15:15-08:00");
    assert_null(st->dt);

    /* phys-address */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang2", "aa:bb:cc:dd");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "aa:bb:cc:dd");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "yang2", "AA:BB:1D:2F:CA:52");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "aa:bb:1d:2f:ca:52");
    lyd_free_withsiblings(st->dt);

    /* mac-address */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang3", "12:34:56:78:9A:BC");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "12:34:56:78:9a:bc");
    lyd_free_withsiblings(st->dt);

    /* hex-string */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang4", "AB:CD:eF:fE:dc:Ba:Ab");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "ab:cd:ef:fe:dc:ba:ab");
    lyd_free_withsiblings(st->dt);

    /* uuid */
    st->dt = lyd_new_leaf(NULL, st->mod, "yang5", "12AbCDef-3456-58cd-9ABC-8796cdACdfEE");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "12abcdef-3456-58cd-9abc-8796cdacdfee");
}

static void
test_inet_types(void **state)
{
    struct state *st = (struct state *)*state;

    /* ip-address */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet1", "192.168.0.1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "192.168.0.1");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet1", "192.168.0.1%12");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "192.168.0.1%12");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet1", "2008:15:0:0:0:0:feAC:1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "2008:15::feac:1");
    lyd_free_withsiblings(st->dt);

    /* ipv6-address */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet2", "FAAC:21:011:Da85::87:daaF%1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "faac:21:11:da85::87:daaf%1");
    lyd_free_withsiblings(st->dt);

    /* ip-address-no-zone */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet3", "127.0.0.1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "127.0.0.1");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet3", "0:00:000:0000:000:00:0:1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "::1");
    lyd_free_withsiblings(st->dt);

    /* ipv6-address-no-zone */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet4", "A:B:c:D:e:f:1:0");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "a:b:c:d:e:f:1:0");
    lyd_free_withsiblings(st->dt);

    /* ip-prefix */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet5", "158.1.58.4/1");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "128.0.0.0/1");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet5", "158.1.58.4/24");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "158.1.58.0/24");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet5", "2000:A:B:C:D:E:f:a/16");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "2000::/16");
    lyd_free_withsiblings(st->dt);

    /* ipv4-prefix */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet6", "0.1.58.4/32");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "0.1.58.4/32");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet6", "12.1.58.4/8");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "12.0.0.0/8");
    lyd_free_withsiblings(st->dt);

    /* ipv6-prefix */
    st->dt = lyd_new_leaf(NULL, st->mod, "inet7", "::C:D:E:f:a/112");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "::c:d:e:f:0/112");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet7", "::C:D:E:f:a/110");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "::c:d:e:c:0/110");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet7", "::C:D:E:f:a/96");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "::c:d:e:0:0/96");
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new_leaf(NULL, st->mod, "inet7", "::C:D:E:f:a/55");
    assert_non_null(st->dt);
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt)->value_str, "::/55");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_yang_types, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_inet_types, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
