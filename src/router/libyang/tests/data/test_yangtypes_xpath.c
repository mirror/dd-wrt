/**
 * @file test_yangtypes_xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for resolving ietf-yang-types type xpath1.0.
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
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
    char *xml;
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
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
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
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_acm_yangtypes_xpath(void **state)
{
    struct state *st = (struct state *)*state;

    /* schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf-acm.yang", LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/data/files/all-imp.yang", LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/data/files/all.yang", LYS_IN_YANG), NULL);

    /* data */
    st->dt = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/nacm.xml", LYD_XML, LYD_OPT_CONFIG /*DATA_NO_YANGLIB*/);
    assert_ptr_not_equal(st->dt, NULL);

    assert_string_equal(((struct lyd_node_leaf_list *)st->dt->child->child->next->child->next)->value_str, "/all:cont1/leaf3");

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\"><rule-list><name>test-list</name><rule><name>test-rule</name><path xmlns:all_mod=\"urn:all\">/all_mod:cont1/all_mod:leaf3</path><action>deny</action></rule></rule-list></nacm>");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_acm_yangtypes_xpath, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
