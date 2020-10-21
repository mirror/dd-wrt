/**
 * @file test_defaults.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for processing default values.
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
    struct lyd_node *dt;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/defaults.yin";
    const char *schema2file = TESTS_DIR"/data/files/defaults2.yang";
    const char *ncwdfile = TESTS_DIR"/schema/yin/ietf/ietf-netconf-with-defaults.yin";
    const char *ietfdir = TESTS_DIR"/schema/yin/ietf/";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(ietfdir, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schemas */
    if (!lys_parse_path(st->ctx, ncwdfile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", ncwdfile);
        goto error;
    }

    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        goto error;
    }

    st->mod2 = lys_parse_path(st->ctx, schema2file, LYS_IN_YANG);
    if (!st->mod2) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schema2file);
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
setup_clean_f(void **state)
{
    struct state *st;
    const char *ncwdfile = TESTS_DIR"/schema/yin/ietf/ietf-netconf-with-defaults.yin";
    const char *ietfdir = TESTS_DIR"/schema/yin/ietf/";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(ietfdir, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schemas */
    if (!lys_parse_path(st->ctx, ncwdfile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", ncwdfile);
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
test_empty(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    st->dt = NULL;
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_status(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<b1_status>42</b1_status>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<papa>42</papa></hidden>";

    st->dt = NULL;
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_DATA | LYD_OPT_DATA_NO_YANGLIB, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_EXPLICIT), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_trim1(void **state)
{
    struct state *st = (*state);
    const char *xml_in = "<df xmlns=\"urn:libyang:tests:defaults\">"
                           "<foo>1</foo><bar><hi>42</hi><ho>1</ho></bar><llist>42</llist>"
                           "<list><name>test</name><value>42</value></list>"
                         "</df>";
    const char *xml_out ="<df xmlns=\"urn:libyang:tests:defaults\"><foo>1</foo><bar><ho>1</ho></bar>"
                         "<list><name>test</name></list></df>";

    assert_ptr_not_equal((st->dt = lyd_parse_mem(st->ctx, xml_in, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_TRIM), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml_out);
}

static void
test_trim2(void **state)
{
    struct state *st = (*state);
    const char *xml_in = "<df xmlns=\"urn:libyang:tests:defaults\">"
                           "<b2>42</b2>"
                         "</df>";

    assert_ptr_not_equal((st->dt = lyd_parse_mem(st->ctx, xml_in, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_TRIM), 0);
    assert_ptr_equal(st->xml, NULL);
}

static void
test_empty_tag(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo>"
                        "<llist ncwd:default=\"true\">42</llist><dllist ncwd:default=\"true\">1</dllist>"
                        "<dllist ncwd:default=\"true\">2</dllist><dllist ncwd:default=\"true\">3</dllist>"
                        "<b1_1 ncwd:default=\"true\">42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    st->dt = NULL;
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df1(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<bar><hi>42</hi><ho>1</ho></bar>"
                        "<foo>42</foo>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);
    /* presence container */
    assert_ptr_not_equal((node = lyd_new(st->dt, NULL, "bar")), NULL);
    assert_int_not_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);
    assert_string_equal(ly_errmsg(st->ctx), "Missing required element \"ho\" in \"bar\".");

    /* manadatory node in bar */
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "ho", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<list><name>a</name><value>42</value></list>"
                        "<list><name>b</name><value>1</value></list>"
                        "<foo>42</foo>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);
    /* lists */
    assert_ptr_not_equal(lyd_new_path(st->dt, NULL, "/defaults:df/defaults:list[name='a']", NULL, 0, 0), NULL);
    assert_ptr_not_equal((node = lyd_new_path(st->dt, NULL, "/defaults:df/defaults:list[name='b']", NULL, 0, 0)), NULL);
    assert_ptr_not_equal(lyd_new_leaf(node, NULL, "value", "1"), NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_df3(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_1>42</b1_1>"
                        "<a1>1</a1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<c><x ncwd:default=\"true\">42</x></c>"
                        "<foo ncwd:default=\"true\">42</foo>"
                        "<llist ncwd:default=\"true\">42</llist><dllist ncwd:default=\"true\">1</dllist>"
                        "<dllist ncwd:default=\"true\">2</dllist><dllist ncwd:default=\"true\">3</dllist>"
                        "<b1_1 ncwd:default=\"true\">42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);

    /* select - c */
    assert_ptr_not_equal((node = lyd_new(st->dt, NULL, "c")), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);

    free(st->xml);
    st->xml = NULL;

    /* select - a */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "a1", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);
}

static void
test_df4(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_2>x</b1_2><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo>"
                        "<llist ncwd:default=\"true\">42</llist><dllist ncwd:default=\"true\">1</dllist>"
                        "<dllist ncwd:default=\"true\">2</dllist><dllist ncwd:default=\"true\">3</dllist>"
                        "<b2>1</b2>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<s2a>1</s2a><foo ncwd:default=\"true\">42</foo>"
                        "<llist ncwd:default=\"true\">42</llist><dllist ncwd:default=\"true\">1</dllist>"
                        "<dllist ncwd:default=\"true\">2</dllist><dllist ncwd:default=\"true\">3</dllist>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\" "
                          "xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                        "<foo ncwd:default=\"true\">42</foo><baz ncwd:default=\"true\">42</baz></hidden>";

    st->dt = lyd_new(NULL, st->mod, "df");
    assert_ptr_not_equal(st->dt, NULL);

    /* select2 - s2a */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "s2a", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml3);

    free(st->xml);
    st->xml = NULL;

    /* select2 - s2b - b2 */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "b2", "1"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);

    free(st->xml);
    st->xml = NULL;

    /* select2 - s2b - b1 */
    assert_ptr_not_equal(lyd_new_leaf(st->dt, NULL, "b1_2", "x"), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);
}

static void
test_rpc_input_default(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<rpc1 xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<inleaf1>hi</inleaf1>"
                         "<inleaf2 ncwd:default=\"true\">def1</inleaf2>"
                       "</rpc1>";
    const char *xml2 = "<rpc1 xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<inleaf2 ncwd:default=\"true\">def1</inleaf2>"
                       "</rpc1>";

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:rpc1/inleaf1[.='hi']", NULL, 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);

    free(st->xml);
    st->xml = NULL;
    lyd_free(st->dt);
    st->dt = NULL;

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:rpc1", NULL, 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPC, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);
}

static void
test_rpc_output_default(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<rpc1 xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<outleaf1 ncwd:default=\"true\">def2</outleaf1>"
                         "<outleaf2>hai</outleaf2>"
                       "</rpc1>";
    const char *xml2 = "<rpc1 xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<outleaf1 ncwd:default=\"true\">def2</outleaf1>"
                       "</rpc1>";

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:rpc1/outleaf2[.='hai']", NULL, 0, LYD_PATH_OPT_OUTPUT);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPCREPLY, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);

    free(st->xml);
    st->xml = NULL;
    lyd_free(st->dt);
    st->dt = NULL;

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:rpc1", NULL, 0, LYD_PATH_OPT_OUTPUT);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_RPCREPLY, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);
}

static void
test_rpc_augment(void **state)
{
    struct state *st = (*state);
    const char *xml = "<oper xmlns=\"urn:defaults2\">"
                         "<c1><c2><l>hi</l></c2></c1>"
                       "</oper>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_RPC, NULL);
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_notif_default(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<notif xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<ntfleaf2>helloo</ntfleaf2>"
                         "<ntfleaf1 ncwd:default=\"true\">def3</ntfleaf1>"
                       "</notif>";
    const char *xml2 = "<notif xmlns=\"urn:libyang:tests:defaults\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\">"
                         "<ntfleaf1 ncwd:default=\"true\">def3</ntfleaf1>"
                       "</notif>";

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:notif/ntfleaf2[.='helloo']", NULL, 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_NOTIF, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml1);

    free(st->xml);
    st->xml = NULL;
    lyd_free(st->dt);
    st->dt = NULL;

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults:notif", NULL, 0, 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_NOTIF, NULL), 0);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL_TAG), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml2);
}

static void
test_val_diff(void **state)
{
    struct state *st = (*state);
    struct lyd_difflist *diff;
    int ret;

    st->dt = lyd_new_path(NULL, st->ctx, "/defaults2:l1[k='when-true']", NULL, 0, 0);
    assert_non_null(st->dt);

    ret = lyd_validate_modules(&st->dt, &st->mod2, 1,  LYD_OPT_CONFIG | LYD_OPT_VAL_DIFF, &diff);
    assert_int_equal(ret, 0);

    assert_int_equal(diff->type[0], LYD_DIFF_CREATED);
    assert_string_equal(diff->second[0]->schema->name, "cont1");
    assert_string_equal(diff->second[0]->child->schema->name, "cont2");
    assert_string_equal(diff->second[0]->child->child->schema->name, "dflt1");
    assert_int_equal(diff->type[1], LYD_DIFF_CREATED);
    assert_string_equal(diff->second[1]->schema->name, "dflt2");
    assert_int_equal(diff->type[2], LYD_DIFF_END);

    lyd_free_val_diff(diff);

    st->dt = st->dt->next;
    lyd_free(st->dt->prev);

    ret = lyd_validate_modules(&st->dt, &st->mod2, 1,  LYD_OPT_CONFIG | LYD_OPT_VAL_DIFF, &diff);
    assert_int_equal(ret, 0);

    assert_int_equal(diff->type[0], LYD_DIFF_DELETED);
    assert_string_equal(diff->first[0]->schema->name, "dflt2");
    assert_int_equal(diff->type[1], LYD_DIFF_END);

    lyd_free_val_diff(diff);
}

static void
test_feature(void **state)
{
    struct state *st = (*state);
    const char *xml = "<hiddenleaf xmlns=\"urn:libyang:tests:defaults\">42"
                      "</hiddenleaf><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><hiddenleaf>42</hiddenleaf>"
                        "<llist>42</llist><dllist>1</dllist><dllist>2</dllist><dllist>3</dllist>"
                        "<b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";

    assert_int_equal(lys_features_enable(st->mod, "unhide"), 0);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);

    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml);
}

static void
test_leaflist_in10(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module x {"
"  namespace \"urn:x\";"
"  prefix x;"
"  leaf-list ll {"
"    type string;"
"    default \"one\";"
"  }}";

    const char *yin = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x\"/>"
"  <prefix value=\"x\"/>"
"  <leaf-list name=\"ll\">"
"    <type name=\"string\"/>"
"    <default value=\"one\"/>"
"  </leaf-list></module>";

    ly_log_options(LY_LOSTORE);
    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_err_first(st->ctx)->vecode, LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_err_first(st->ctx)->prev->prev->vecode, LYVE_INSTMT);
    ly_err_clean(st->ctx, NULL);
    ly_log_options(LY_LOLOG | LY_LOSTORE_LAST);
}

static void
test_leaflist_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/>"
"  <prefix value=\"x\"/>"
"  <leaf-list name=\"ll\">"
"    <type name=\"string\"/>"
"    <default value=\"one\"/>"
"    <default value=\"two\"/>"
"  </leaf-list></module>";

    const char *xml_empty = "<ll xmlns=\"urn:x\">one</ll><ll xmlns=\"urn:x\">two</ll>";

    const char *xml_one = "<ll xmlns=\"urn:x\">one</ll>";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    st->dt = NULL;
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml_empty);

    free(st->xml);
    lyd_free_withsiblings(st->dt);

    assert_ptr_not_equal(st->dt = lyd_new_path(NULL, st->ctx, "/x:ll", "one", 0, 0), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml_one);
}

static void
test_leaflist_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  leaf-list ll {"
"    type string;"
"    default \"one\";"
"    default \"two\";"
"  }}";
    const char *xml_empty = "<ll xmlns=\"urn:x\">one</ll><ll xmlns=\"urn:x\">two</ll>";

    const char *xml_three = "<ll xmlns=\"urn:x\">three</ll>";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->dt = NULL;
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml_empty);

    free(st->xml);
    lyd_free_withsiblings(st->dt);

    assert_ptr_not_equal(st->dt = lyd_new_path(NULL, st->ctx, "/x:ll", "three", 0, 0), NULL);
    assert_int_equal(lyd_validate(&(st->dt), LYD_OPT_CONFIG, st->ctx), 0);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(lyd_print_mem(&(st->xml), st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_WD_ALL), 0);
    assert_ptr_not_equal(st->xml, NULL);
    assert_string_equal(st->xml, xml_three);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_empty, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_empty_tag, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_status, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_trim1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_trim2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df3, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_df4, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_rpc_input_default, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_rpc_output_default, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_rpc_augment, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_notif_default, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_val_diff, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_feature, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leaflist_in10, setup_clean_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leaflist_yang, setup_clean_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leaflist_yin, setup_clean_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
