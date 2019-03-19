/**
 * @file test_must_1.1.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for resolving YANG 1.1 must-stmt constraints.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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
    struct lyd_node *dt2;
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
    lyd_free_withsiblings(st->dt2);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_dependency_rpc(void **state)
{
    struct state *st = (struct state *)*state;

    /* schema */
    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/must-dependrpc.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);
    if (!(st->mod->data->next->child->child->flags & LYS_XPCONF_DEP)) {
        fail();
    }

    st->dt = lyd_new_path(NULL, st->ctx, "/must-dependrpc:rpc1/b", "val_b", 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    st->dt2 = lyd_new_path(NULL, st->ctx, "/must-dependrpc:top/a", "val_a", 0, 0);
    assert_ptr_not_equal(st->dt2, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, st->dt2), 0);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<rpc1 xmlns=\"urn:libyang:tests:must-dependrpc\"><b>val_b</b></rpc1>");
    free(st->xml);
    lyd_print_mem(&(st->xml), st->dt2, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<top xmlns=\"urn:libyang:tests:must-dependrpc\"><a>val_a</a></top>");
}

static void
test_dependency_action(void **state)
{
    struct state *st = (struct state *)*state;

    /* schema */
    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/must-dependact.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);
    if (!(st->mod->data->child->child->next->next->next->child->child->flags & LYS_XPCONF_DEP)) {
        fail();
    }

    st->dt = lyd_new_path(NULL, st->ctx, "/must-dependact:top/list1[key1='a'][key2='b']/act1/b", "bb", 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    st->dt2 = lyd_new_path(NULL, st->ctx, "/must-dependact:top/list1[key1='c'][key2='d']/a", "aa", 0, 0);
    assert_ptr_not_equal(st->dt2, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, st->dt2), 0);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_NETCONF);
    assert_string_equal(st->xml, "<action xmlns=\"urn:ietf:params:xml:ns:yang:1\"><top xmlns=\"urn:libyang:tests:must-dependact\"><list1><key1>a</key1><key2>b</key2><act1><b>bb</b></act1></list1></top></action>");
    free(st->xml);
    lyd_print_mem(&(st->xml), st->dt2, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<top xmlns=\"urn:libyang:tests:must-dependact\"><list1><key1>c</key1><key2>d</key2><a>aa</a></list1></top>");
}

static void
test_inout(void **state)
{
    struct state *st = (struct state *)*state;
    struct lyd_node *node;

    /* schema */
    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/must-inout.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);

    /* input */
    st->dt = lyd_new_path(NULL, st->ctx, "/must-inout:rpc1/b", "bb", 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, NULL), 1);

    node = lyd_new_path(st->dt, st->ctx, "/must-inout:rpc1/c", "5", 0, 0);
    assert_ptr_not_equal(node, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, NULL), 0);

    /* output */
    st->dt2 = lyd_new_path(NULL, st->ctx, "/must-inout:rpc1/d", "0", 0, LYD_PATH_OPT_OUTPUT);
    assert_ptr_not_equal(st->dt2, NULL);

    assert_int_equal(lyd_validate(&(st->dt2), LYD_OPT_RPCREPLY, NULL), 1);

    node = lyd_new_path(st->dt2, st->ctx, "/must-inout:rpc1/d", "6", 0, LYD_PATH_OPT_OUTPUT | LYD_PATH_OPT_UPDATE);
    assert_ptr_not_equal(node, NULL);

    assert_int_equal(lyd_validate(&(st->dt2), LYD_OPT_RPCREPLY, NULL), 0);
}

static void
test_notif(void **state)
{
    struct state *st = (struct state *)*state;
    struct lyd_node *node;

    /* schema */
    st->mod = lys_parse_path(st->ctx, TESTS_DIR"/data/files/must-notif.yin", LYS_IN_YIN);
    assert_ptr_not_equal(st->mod, NULL);

    /* notif */
    st->dt = lyd_new_path(NULL, st->ctx, "/must-notif:notif1/b", "bb", 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_NOTIF, NULL), 1);

    node = lyd_new_path(st->dt, st->ctx, "/must-notif:notif1/a", "5", 0, 0);
    assert_ptr_not_equal(node, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_NOTIF, NULL), 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_dependency_rpc, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_dependency_action, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_inout, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_notif, setup_f, teardown_f)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
