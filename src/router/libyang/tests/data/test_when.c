/**
 * @file test_when.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for resolving when-stmt constraints.
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
    const struct lys_module *mod2;
    const struct lys_module *mod3;
    struct lyd_node *dt;
    struct lyd_node *act;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/when.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf", 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
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
    lyd_free_withsiblings(st->act);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_parse(void **state)
{
    struct state *st = (*state);
    const char *xml = "<top xmlns=\"urn:libyang:tests:when\"><a>A</a><b><b1>B</b1></b><c>C</c></top>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, xml);
}

static void
test_netconf_autodel(void **state)
{
    const char *schema = TESTS_DIR"/data/files/nc-when.yang";
    const struct lys_module *mod;
    struct state *st = (*state);
    struct lyd_node *node;
    int ret;

    /* load special schema for this test */
    mod = lys_parse_path(st->ctx, schema, LYS_YANG);
    assert_non_null(mod);

    /* create valid data tree */
    st->dt = lyd_new_path(NULL, st->ctx, "/nc-when:test-when/when-check", "true", 0, 0);
    assert_non_null(st->dt);
    node = lyd_new_path(st->dt, NULL, "/nc-when:test-when/gated-data", "100", 0, 0);
    assert_non_null(node);
    ret = lyd_validate(&st->dt, LYD_OPT_CONFIG | LYD_OPT_STRICT | LYD_OPT_WHENAUTODEL, NULL);
    assert_int_equal(ret, 0);

    /*
     * Change when to false and auto-delete the conditioned node during validation.
     * This is the only case when a node should be silently deleted (provided that the flag is used).
     */
    assert_non_null(st->dt->child->next);

    node = st->dt->child;
    assert_string_equal(node->schema->name, "when-check");
    ret = lyd_change_leaf((struct lyd_node_leaf_list *)node, "false");
    assert_int_equal(ret, 0);
    ret = lyd_validate(&st->dt, LYD_OPT_CONFIG | LYD_OPT_STRICT | LYD_OPT_WHENAUTODEL, NULL);
    assert_int_equal(ret, 0);

    assert_null(st->dt->child->next);

    /*
     * If we try to create the deleted node now, we must get an error despite using the auto-delete flag.
     * libyang must be able to handle this situation internally because these 2 cases may not be detectable
     * in an application.
     */
    node = lyd_new_path(st->dt, NULL, "/nc-when:test-when/gated-data", "100", 0, 0);
    assert_non_null(node);
    ret = lyd_validate(&st->dt, LYD_OPT_CONFIG | LYD_OPT_STRICT | LYD_OPT_WHENAUTODEL, NULL);
    assert_int_equal(ret, 1);

    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOWHEN);
    assert_string_equal(ly_errpath(st->ctx), "/nc-when:test-when/gated-data");
}

static void
test_parse_noautodel(void **state)
{
    struct state *st = (*state);
    const char *xml = "<top xmlns=\"urn:libyang:tests:when\"><b><b1>B</b1></b><c>C</c></top>";

    /* when parsing data, false when is always an error */
    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_WHENAUTODEL);
    assert_null(st->dt);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOWHEN);
    assert_string_equal(ly_errpath(st->ctx), "/when:top/c");

    xml = "<topleaf xmlns=\"urn:libyang:tests:when\">X</topleaf>"
          "<top xmlns=\"urn:libyang:tests:when\"><b><b1>B</b1></b><c>C</c></top>";
    lyd_free_withsiblings(st->dt);

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_WHENAUTODEL);
    assert_null(st->dt);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOWHEN);
    assert_string_equal(ly_errpath(st->ctx), "/when:top/c");
}

static void
test_insert(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;

    st->dt = lyd_new(NULL, st->mod, "top");
    assert_ptr_not_equal(st->dt, NULL);

    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "c", "C"), NULL);
    node = lyd_new(st->dt, NULL, "b");
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "b1", "B"), NULL);
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "a", "A"), NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    lyd_print_mem(&(st->xml), st->dt, LYD_XML, 0);
    assert_string_equal(st->xml, "<top xmlns=\"urn:libyang:tests:when\"><c>C</c><b><b1>B</b1></b><a>A</a></top>");
}

static void
test_insert_noautodel(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;

    st->dt = lyd_new(NULL, st->mod, "top");
    assert_ptr_not_equal(st->dt, NULL);

    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "c", "C"), NULL);
    node = lyd_new(st->dt, NULL, "b");
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "b1", "B"), NULL);

    /* when is not changing from true to false, always an error */
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG | LYD_OPT_WHENAUTODEL, NULL), 1);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOWHEN);
    assert_string_equal(ly_errpath(st->ctx), "/when:top/c");

    lyd_free_withsiblings(st->dt);

    st->dt = lyd_new(NULL, st->mod, "top");
    assert_ptr_not_equal(st->dt, NULL);

    node = lyd_new_leaf(NULL, st->mod, "topleaf", "X");
    assert_ptr_not_equal(node, NULL);
    assert_int_equal(lyd_insert_after(st->dt, node), 0);

    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "c", "C"), NULL);
    node = lyd_new(st->dt, NULL, "b");
    assert_ptr_not_equal(node, NULL);
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "b1", "B"), NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG | LYD_OPT_WHENAUTODEL, NULL), 1);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOWHEN);
    assert_string_equal(ly_errpath(st->ctx), "/when:top/c");
}

static void
test_value_prefix(void **state)
{
    struct state *st = (struct state *)*state;

    /* schema */
    st->mod2 = lys_parse_path(st->ctx, TESTS_DIR"/data/files/when-value-prefix.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod2, NULL);
    st->mod3 = lys_parse_path(st->ctx, TESTS_DIR"/data/files/when-value-prefix-aug.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod3, NULL);

    st->dt = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/when-value-prefix.xml", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_STRICT | LYD_OPT_CONFIG, NULL), 0);
    lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(st->xml, "<outer xmlns=\"urn:when:value:prefix\"><indicator xmlns:wvpa=\"urn:when:value:prefix:aug\">wvpa:inner-indicator</indicator><inner xmlns=\"urn:when:value:prefix:aug\"><text>any-text</text></inner></outer>");
}

static void
test_augment_choice(void **state)
{
    struct state *st = (struct state *)*state;
    const char data[] =
"<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
    "<interface>"
        "<name>bu</name>"
        "<type xmlns:ii=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ii:ethernetCsmacd</type>"
    "</interface>"
"</interfaces>";
    const char *schemafile = TESTS_DIR"/data/files/ietf-microwave-radio-link@2018-10-03.yang";

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    st->mod2 = lys_parse_path(st->ctx, schemafile, LYS_IN_YANG);
    assert_non_null(st->mod2);

    st->mod3 = ly_ctx_get_module(st->ctx, "iana-if-type", NULL, 0);
    assert_non_null(st->mod3);
    lys_set_implemented(st->mod3);

    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_non_null(st->dt);
}

static void
test_action(void **state)
{
    struct state *st = (struct state *)*state;
    const char act[] =
        "<advanced xmlns=\"urn:act1\">"
            "<conditional xmlns=\"urn:act2\">"
                "<conditional_action/>"
            "</conditional>"
        "</advanced>";
    const char data[] =
        "<advanced xmlns=\"urn:act1\">"
            "<condition>true</condition>"
            "<conditional xmlns=\"urn:act2\">"
                "<b_positive>25</b_positive>"
            "</conditional>"
        "</advanced>";

    /* schema */
    st->mod2 = lys_parse_path(st->ctx, TESTS_DIR"/data/files/act1.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod2, NULL);
    assert_int_equal(lys_features_enable(st->mod2, "feat1"), 0);
    st->mod3 = lys_parse_path(st->ctx, TESTS_DIR"/data/files/act2.yang", LYS_IN_YANG);
    assert_ptr_not_equal(st->mod3, NULL);

    st->dt = lyd_parse_mem(st->ctx, data, LYD_XML, LYD_OPT_CONFIG);
    assert_non_null(st->dt);

    st->act = lyd_parse_mem(st->ctx, act, LYD_XML, LYD_OPT_RPC, st->dt);
    assert_non_null(st->act);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_parse, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_netconf_autodel, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_parse_noautodel, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_insert, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_insert_noautodel, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_value_prefix, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_augment_choice, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_action, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
