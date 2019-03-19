/**
 * @file test_metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for ietf-yang-metadata (annotations extension).
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
    struct lyd_node *data;
    char *str;
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
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf/", 0);
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

    free(st->str);
    lyd_free_withsiblings(st->data);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

/*
 * leafref is not supported for annotations in libyang
 */
static void
test_leafref_type(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  import ietf-yang-metadata { prefix md; }"
                    "  md:annotation x { type leafref { path \"/x:a\"; } }"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_errno, LY_EPLUGIN);
}

/*
 * attribute with no appropriate anotation specification cannot be loaded
 */
static void
test_unknown_metadata_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:x=\"urn:x\" x:attribute=\"not-defined\">a</a>";

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* parse input with strict - error */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);

    /* parse input without strict - passes, but the attribute is not present */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_CONFIG, NULL);
    assert_ptr_not_equal(st->data, NULL);
    assert_ptr_equal(st->data->attr, NULL);
}

static void
test_unknown_metadata_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\": \"a\","
        "\"@x:a\": {"
            "\"x:attribute\": \"not-defined\""
        "}"
    "}";

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* parse input with strict - error */
    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_CONFIG | LYD_OPT_STRICT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INMETA);

    /* parse input without strict - passes, but the attribute is not present */
    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_CONFIG, NULL);
    assert_ptr_not_equal(st->data, NULL);
    assert_ptr_equal(st->data->attr, NULL);
}

/*
 * correctness of parsing and printing NETCONF's filter's attributes
 */
static void
test_nc_filter1_xml(void **state)
{
    struct state *st = (*state);
    const char *filter_subtree = "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
                    "<filter type=\"subtree\"><modules-state "
                    "xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\"><module-set-id/>"
                    "</modules-state></filter></get>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, filter_subtree, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, filter_subtree);
}

static void
test_nc_filter2_xml(void **state)
{
    const struct lys_module *mod;
    struct state *st = (*state);
    const char *filter_xpath = "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
                    "<filter type=\"xpath\" "
                    "xmlns:yanglib=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\" "
                    "select=\"/yanglib:modules-state/yanglib:module-set-id\"/></get>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal((mod = lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG)), NULL);
    assert_int_equal(lys_features_enable(mod, "xpath"), 0);

    st->data = lyd_parse_mem(st->ctx, filter_xpath, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, filter_xpath);
}

/*
 * correctness of parsing and printing NETCONF's filter attributes
 * - incorrect namespace
 */
static void
test_nc_filter3_xml(void **state)
{
    struct state *st = (*state);
    const char *input =
        "<get-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.1\">"
          "<source>"
            "<running/>"
          "</source>"
          "<filter nc:type=\"subtree\">"
            "<some>filter</some>"
          "</filter>"
        "</get-config>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC | LYD_OPT_STRICT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->data, NULL);
    assert_ptr_equal(st->data->child->next->attr, NULL);
}

/*
 * correctness of parsing and printing NETCONF's filter attributes
 * - incorrect value
 */
static void
test_nc_filter4_xml(void **state)
{
    struct state *st = (*state);
    const char *input =
        "<get-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
          "<source>"
            "<running/>"
          "</source>"
          "<filter type=\"subtrees\">"
            "<some>filter</some>"
          "</filter>"
        "</get-config>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INMETA);
}

/*
 * correctness of parsing and printing NETCONF's filter attributes
 * - xpath feature off
 */
static void
test_nc_filter5_xml(void **state)
{
    struct state *st = (*state);
    const char *input =
        "<get-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
          "<source>"
            "<running/>"
          "</source>"
          "<filter type=\"xpath\">"
            "<some>filter</some>"
          "</filter>"
        "</get-config>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INMETA);
}

/*
 * correctness of parsing and printing NETCONF's filter attributes
 * - mix of filter types
 */
static void
test_nc_filter6_xml(void **state)
{
    const struct lys_module *mod;
    struct state *st = (*state);
    const char *input =
        "<get-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
          "<source>"
            "<running/>"
          "</source>"
          "<filter type=\"subtree\" type=\"xpath\" select=\"/*[local-name() = 'modules-state']/*\">"
          "</filter>"
        "</get-config>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal((mod = lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG)), NULL);
    assert_int_equal(lys_features_enable(mod, "xpath"), 0);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's filter attributes
 * - xpath filter without select
 */
static void
test_nc_filter7_xml(void **state)
{
    const struct lys_module *mod;
    struct state *st = (*state);
    const char *input =
        "<get-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
          "<source>"
            "<running/>"
          "</source>"
          "<filter type=\"xpath\">"
          "</filter>"
        "</get-config>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal((mod = lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG)), NULL);
    assert_int_equal(lys_features_enable(mod, "xpath"), 0);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - invalid operation's value
 */
static void
test_nc_editconfig1_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"not-defined\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* operation attribute is valid, but its value is invalid so the parsing fails no matter if strict is used */
    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INMETA);
}

static void
test_nc_editconfig1_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":\"a\","
        "\"@x:a\":{"
            "\"ietf-netconf:operation\":\"not-defined\""
        "}"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    /* operation attribute is valid, but its value is invalid so the parsing fails no matter if strict is used */
    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INMETA);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - too many operation attributes
 */
static void
test_nc_editconfig2_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"merge\" nc:operation=\"replace\" nc:operation=\"create\" "
                    "nc:operation=\"delete\" nc:operation=\"remove\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_nc_editconfig2_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":\"a\","
        "\"@x:a\":{"
            "\"ietf-netconf:operation\":\"merge\","
            "\"ietf-netconf:operation\":\"replace\","
            "\"ietf-netconf:operation\":\"create\","
            "\"ietf-netconf:operation\":\"delete\","
            "\"ietf-netconf:operation\":\"remove\""
        "}"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig3_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                    "nc:operation=\"create\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

static void
test_nc_editconfig3_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":\"a\","
        "\"@x:a\":{"
            "\"ietf-netconf:operation\":\"create\""
        "}"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_JSON, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - insert, value attr in system-ordered list
 */
static void
test_nc_editconfig4_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:insert=\"before\" yang:value=\"b\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig4_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"yang:insert\":\"before\","
            "\"yang:value\":\"b\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - insert attr in operation delete
 */
static void
test_nc_editconfig5_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"delete\" yang:insert=\"first\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig5_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"delete\","
            "\"yang:insert\":\"first\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - several insert attr
 */
static void
test_nc_editconfig6_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"first\" yang:insert=\"last\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_nc_editconfig6_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"create\","
            "\"yang:insert\":\"first\","
            "\"yang:insert\":\"last\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - several value attr
 */
static void
test_nc_editconfig7_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:value=\"b\" yang:value=\"d\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_nc_editconfig7_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"create\","
            "\"yang:insert\":\"before\","
            "\"yang:value\":\"b\","
            "\"yang:value\":\"d\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig8_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"create\" yang:insert=\"first\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

static void
test_nc_editconfig8_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"create\","
            "\"yang:insert\":\"first\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_JSON, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig9_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"replace\" yang:insert=\"before\" yang:value=\"b\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

static void
test_nc_editconfig9_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"replace\","
            "\"yang:insert\":\"before\","
            "\"yang:value\":\"b\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_JSON, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - insert in system ordered list
 */
static void
test_nc_editconfig10_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by system; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:key=\"[...]\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig10_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by system; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"create\","
                "\"yang:insert\":\"before\","
                "\"yang:key\":\"[...]\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - insert attr in operation delete
 */
static void
test_nc_editconfig11_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"delete\" yang:insert=\"first\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig11_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"delete\","
                "\"yang:insert\":\"first\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - several insert attr
 */
static void
test_nc_editconfig12_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:insert=\"last\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_nc_editconfig12_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"create\","
                "\"yang:insert\":\"before\","
                "\"yang:insert\":\"last\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT , NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - several key attr
 */
static void
test_nc_editconfig13_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:key=\"[...]\" yang:key=\"[...]\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

static void
test_nc_editconfig13_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"create\","
                "\"yang:insert\":\"before\","
                "\"yang:key\":\"[...]\","
                "\"yang:key\":\"[...]\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_TOOMANY);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig14_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"create\" yang:insert=\"first\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

static void
test_nc_editconfig14_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"create\","
                "\"yang:insert\":\"first\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_JSON, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - correct use
 */
static void
test_nc_editconfig15_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"replace\" yang:insert=\"before\" yang:key=\"[...]\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_XML, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

static void
test_nc_editconfig15_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"replace\","
                "\"yang:insert\":\"before\","
                "\"yang:key\":\"[...]\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_not_equal(st->data, NULL);
    lyd_print_mem(&st->str, st->data, LYD_JSON, 0);
    assert_ptr_not_equal(st->str, NULL);
    assert_string_equal(st->str, input);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - value in list
 */
static void
test_nc_editconfig16_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:value=\"d\"><k>a</k></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig16_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  list a { key \"k\"; leaf k { type string; } ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[{"
            "\"@\":{"
                "\"ietf-netconf:operation\":\"create\","
                "\"yang:insert\":\"before\","
                "\"yang:value\":\"d\""
            "},"
            "\"k\":\"a\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - key in leaf-list
 */
static void
test_nc_editconfig17_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"create\" yang:insert=\"before\" yang:key=\"[...]\">a</a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

static void
test_nc_editconfig17_json(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf-list a { type string; ordered-by user; }"
                    "}";
    const struct lys_module *mod;
    const char *input = "{"
        "\"x:a\":[\"a\"],"
        "\"@x:a\":[{"
            "\"ietf-netconf:operation\":\"create\","
            "\"yang:insert\":\"before\","
            "\"yang:key\":\"[...]\""
        "}]"
    "}";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_JSON, LYD_OPT_EDIT, NULL);
    assert_ptr_equal(st->data, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INATTR);
}

/*
 * correctness of parsing and printing NETCONF's edit-config's attributes
 * - operation delete with an empty XML tag
 */
static void
test_nc_editconfig18_xml(void **state)
{
    struct state *st = (*state);
    const char *yang = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf a { type string; }"
                    "}";
    const struct lys_module *mod;
    const char *input =
        "<a xmlns=\"urn:x\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
            "nc:operation=\"delete\"></a>";

    /* load ietf-netconf schema */
    assert_ptr_not_equal(lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/ietf/ietf-netconf.yang", LYS_IN_YANG), NULL);

    /* load schema */
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->data = lyd_parse_mem(st->ctx, input, LYD_XML, LYD_OPT_EDIT , NULL);
    assert_ptr_not_equal(st->data, NULL);
    assert_ptr_not_equal(st->data->attr, NULL);
    assert_ptr_not_equal(st->data->attr->name, NULL);
    assert_ptr_not_equal(st->data->attr->value_str, NULL);
    assert_string_equal(st->data->attr->name, "operation");
    assert_string_equal(st->data->attr->value_str, "delete");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_leafref_type, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_unknown_metadata_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_unknown_metadata_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter1_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter2_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter3_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter4_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter5_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter6_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_filter7_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig1_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig1_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig2_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig2_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig3_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig3_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig4_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig4_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig5_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig5_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig6_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig6_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig7_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig7_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig8_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig8_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig9_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig9_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig10_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig10_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig11_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig11_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig12_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig12_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig13_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig13_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig14_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig14_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig15_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig15_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig16_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig16_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig17_xml, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig17_json, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_nc_editconfig18_xml, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
