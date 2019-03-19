/**
 * @file test_tree_data_merge.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for complex data merges.
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx1;
    struct ly_ctx *ctx2;
    struct ly_ctx *ctx3;
    struct lyd_node *source;
    struct lyd_node *target;
    struct lyd_node *result;
    char *path, *output;
};

static int
setup_dflt(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    st->ctx1 = ly_ctx_new(TESTS_DIR "/api/files", 0);
    if (!st->ctx1) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx1, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_dflt(void **state)
{
    struct state *st = (*state);

    free(st->path);
    free(st->output);
    lyd_free_withsiblings(st->target);
    lyd_free_withsiblings(st->source);
    lyd_free_withsiblings(st->result);
    ly_ctx_destroy(st->ctx1, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static int
setup_mctx(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx1 = ly_ctx_new(NULL, 0);
    st->ctx2 = ly_ctx_new(NULL, 0);
    st->ctx3 = ly_ctx_new(NULL, 0);
    if (!st->ctx1 || !st->ctx2 || !st->ctx3) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    return 0;
}

static int
teardown_mctx(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->source);
    lyd_free_withsiblings(st->target);
    lyd_free_withsiblings(st->result);
    ly_ctx_destroy(st->ctx1, NULL);
    ly_ctx_destroy(st->ctx2, NULL);
    ly_ctx_destroy(st->ctx3, NULL);

    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_merge(void **state)
{
    struct state *st = (*state);
    uint32_t i;
    const char *output_template =
"<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
  "<module>"
    "<name>yang</name>"
    "<revision>2016-02-11</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-yang-library</name>"
    "<revision>2016-02-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-acm</name>"
    "<revision>2012-02-22</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf</name>"
    "<revision>2011-06-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
    "<feature>writable-running</feature>"
    "<feature>candidate</feature>"
    "<feature>rollback-on-error</feature>"
    "<feature>validate</feature>"
    "<feature>startup</feature>"
    "<feature>xpath</feature>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-monitoring</name>"
    "<revision>2010-10-04</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
  "</module>"
  "<module>"
    "<name>ietf-netconf-with-defaults</name>"
    "<revision>2011-06-01</revision>"
    "<conformance-type>implement</conformance-type>"
    "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
  "</module>"
"</modules-state>";

    st->target = lyd_parse_path(st->ctx1, TESTS_DIR "/api/files/merge_start.xml", LYD_XML, LYD_OPT_GET);
    assert_ptr_not_equal(st->target, NULL);

    asprintf(&st->path, TESTS_DIR "/api/files/mergeXX.xml");
    for (i = 1; i < 12; ++i) {
        sprintf(st->path + (strlen(st->path) - 6), "%02u.xml", i);
        st->source = lyd_parse_path(st->ctx1, st->path, LYD_XML, LYD_OPT_GET);
        assert_ptr_not_equal(st->source, NULL);

        assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_DESTRUCT), 0);
        st->source = NULL;
    }

    lyd_print_mem(&st->output, st->target, LYD_XML, 0);
    assert_string_equal(st->output, output_template);
}

static void
test_merge2(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }"
                    "  container c1 {"
                    "    container c2 {"
                    "      leaf y { type string; }"
                    "}}}";
    const char *trg = "<x xmlns=\"urn:x\">x</x>";
    const char *result = "<x xmlns=\"urn:x\">x</x><c1 xmlns=\"urn:x\"><c2><y>y</y></c2></c1>";
    char *printed = NULL;

    /* merging leaf x and leaf y - without the parents, lyd_merge is supposed also to merge subtrees */
    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);
    st->result = lyd_new_path(NULL, st->ctx1, "/x:c1/c2/y", "y", 0, 0);
    assert_ptr_not_equal(st->result, NULL);
    assert_int_equal(lyd_validate(&st->result, LYD_OPT_CONFIG, NULL), 0);

    /* lyd_new_path returns the first created node, so the top level c1, but we need the
     * subtree with the y node */
    st->source = st->result->child->child;
    lyd_unlink(st->source);
    lyd_free(st->result);
    st->result = NULL;

    /* the target tree contains only the x node */
    st->target = lyd_parse_mem(st->ctx1, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    /* merge them */
    assert_int_equal(lyd_merge(st->target, st->source, 0), 0);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge3(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "    container A {"
                    "      leaf f1 {type string;}"
                    "      container B {"
                    "        leaf f2 {type string;}"
                    "      }"
                    "    }"
                    "  }";
    const char *trg = "<A xmlns=\"urn:x\"> <f1>block</f1> </A>";
    const char *src = "<A xmlns=\"urn:x\"> <f1>aa</f1> <B> <f2>bb</f2> </B> </A>";
    const char *result = "<A xmlns=\"urn:x\"><f1>aa</f1><B><f2>bb</f2></B></A>";
    char *printed = NULL;

    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx1, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    /* merge them */
    assert_int_equal(lyd_merge(st->target, st->source, 0), 0);
    assert_int_equal(lyd_validate(&st->target, LYD_OPT_CONFIG, NULL), 0);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge4(void **state)
{
    struct state *st = (*state);
    const char *sch =
        "module A {"
            "namespace \"aa:A\";"
            "prefix A;"
            "container A {"
                "leaf f1 {type string;}"
                "container B {"
                    "leaf f2 {type string;}"
                "}"
                "container C {"
                    "leaf f3 {type string;}"
                "}"
            "}"
        "}";

    const char *trg = "<A xmlns=\"aa:A\"> <B> <f2>aaa</f2> </B> </A>";
    const char *src = "<A xmlns=\"aa:A\"> <C> <f3>bbb</f3> </C> </A>";
    const char *result = "<A xmlns=\"aa:A\"><B><f2>aaa</f2></B><C><f3>bbb</f3></C></A>";
    char *printed = NULL;

    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx1, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    /* merge them */
    assert_int_equal(lyd_merge(st->target, st->source, 0), 0);
    assert_int_equal(lyd_validate(&st->target, LYD_OPT_CONFIG, NULL), 0);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge5(void **state)
{
    struct state *st = (*state);
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"

        "container inner1 {"
            "list b-list1 {"
                "key p1;"
                "leaf p1 {"
                    "type uint8;"
                "}"
                "leaf p2 {"
                    "type string;"
                "}"
                "leaf p3 {"
                    "type boolean;"
                    "default false;"
                "}"
            "}"
        "}"
    "}";


    const char *trg =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>a</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";
    const char *src =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
        "</b-list1>"
    "</inner1>";
    const char *result =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";
    char *printed = NULL;

    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx1, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    /* merge them */
    assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_EXPLICIT), 0);
    assert_int_equal(lyd_validate(&st->target, LYD_OPT_CONFIG, NULL), 0);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge_dflt1(void **state)
{
    struct state *st = (*state);
    struct lyd_node *tmp;

    assert_ptr_not_equal(ly_ctx_load_module(st->ctx1, "merge-dflt", NULL), NULL);

    st->target = lyd_new_path(NULL, st->ctx1, "/merge-dflt:top/c", "c_dflt", 0, 0);
    assert_ptr_not_equal(st->target, NULL);
    assert_int_equal(lyd_validate(&(st->target), LYD_OPT_CONFIG, NULL), 0);

    st->source = lyd_new_path(NULL, st->ctx1, "/merge-dflt:top/a", "a_val", 0, 0);
    assert_ptr_not_equal(st->source, NULL);
    tmp = lyd_new_path(st->source, st->ctx1, "/merge-dflt:top/b", "b_val", 0, 0);
    assert_ptr_not_equal(tmp, NULL);
    assert_int_equal(lyd_validate(&(st->source), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_DESTRUCT), 0);
    st->source = NULL;

    /* c should be replaced and now be default */
    assert_int_equal(st->target->child->dflt, 1);
}

static void
test_merge_dflt2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *tmp;

    assert_ptr_not_equal(ly_ctx_load_module(st->ctx1, "merge-dflt", NULL), NULL);

    st->target = lyd_new_path(NULL, st->ctx1, "/merge-dflt:top/c", "c_dflt", 0, 0);
    assert_ptr_not_equal(st->target, NULL);
    assert_int_equal(lyd_validate(&(st->target), LYD_OPT_CONFIG, NULL), 0);

    st->source = lyd_new_path(NULL, st->ctx1, "/merge-dflt:top/a", "a_val", 0, 0);
    assert_ptr_not_equal(st->source, NULL);
    tmp = lyd_new_path(st->source, st->ctx1, "/merge-dflt:top/b", "b_val", 0, 0);
    assert_ptr_not_equal(tmp, NULL);
    assert_int_equal(lyd_validate(&(st->source), LYD_OPT_CONFIG, NULL), 0);

    assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_EXPLICIT), 0);

    /* c should not be replaced, so c remains not default */
    assert_int_equal(st->target->child->dflt, 0);
}

static void
test_merge_to_trgctx1(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }"
                    "  leaf y { type string; }}";
    const char *src = "<x xmlns=\"urn:x\">x</x>";
    const char *trg = "<y xmlns=\"urn:x\">y</y>";
    const char *result = "<y xmlns=\"urn:x\">y</y><x xmlns=\"urn:x\">x</x>";
    char *printed = NULL;

    /* case 1: src is in ctx1, trg is in ctx2, result is expected in ctx2, src is being destroyed */
    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx2, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx2, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_DESTRUCT), 0);
    /* forget pointer to source, it should be destroyed, if not it will be seen in valgrind as memory leak */
    st->source = NULL;

    assert_ptr_equal(st->target->schema->module->ctx, st->ctx2);
    /* check the merged data - leaf x */
    assert_ptr_not_equal(st->target->next, NULL);
    assert_ptr_equal(st->target->next->schema->module->ctx, st->ctx2);

    /* print the result after freeing the ctx1 */
    ly_ctx_destroy(st->ctx1, NULL);
    st->ctx1 = NULL;

    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge_to_trgctx2(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }"
                    "  leaf y { type string; }}";
    const char *src = "<x xmlns=\"urn:x\">x</x>";
    const char *trg = "<y xmlns=\"urn:x\">y</y>";
    const char *result = "<y xmlns=\"urn:x\">y</y><x xmlns=\"urn:x\">x</x>";
    char *printed = NULL;

    /* case 2: src is in ctx1, trg is in ctx2, result is expected in ctx2, src is not destroyed */
    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx2, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx2, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    assert_int_equal(lyd_merge(st->target, st->source, 0), 0);

    assert_ptr_equal(st->target->schema->module->ctx, st->ctx2);
    /* check the merged data - leaf x */
    assert_ptr_not_equal(st->target->next, NULL);
    assert_ptr_equal(st->target->next->schema->module->ctx, st->ctx2);

    assert_ptr_not_equal(st->source, st->target->next);

    /* print the result after freeing the ctx1 */
    lyd_free(st->source);
    ly_ctx_destroy(st->ctx1, NULL);
    st->source = NULL;
    st->ctx1 = NULL;

    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge_to_ctx(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }"
                    "  leaf y { type string; }}";
    const char *src = "<x xmlns=\"urn:x\">x</x>";
    const char *trg = "<y xmlns=\"urn:x\">y</y>";
    const char *result = "<y xmlns=\"urn:x\">y</y><x xmlns=\"urn:x\">x</x>";
    char *printed = NULL;

    /* case 3: src is in ctx1, trg is in ctx2, result is requested in ctx3, src is destroyed */
    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx2, sch, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx3, sch, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx2, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    assert_int_equal(lyd_merge_to_ctx(&st->target, st->source, LYD_OPT_DESTRUCT, st->ctx3), 0);
    st->source = NULL; /* source is expected to be destroyed */

    /* check the merged data - leaf y */
    assert_ptr_equal(st->target->schema->module->ctx, st->ctx3);
    /* check the merged data - leaf x */
    assert_ptr_not_equal(st->target->next, NULL);
    assert_ptr_equal(st->target->next->schema->module->ctx, st->ctx3);

    /* print the result after freeing the ctx1 and ctx2 */
    ly_ctx_destroy(st->ctx1, NULL);
    ly_ctx_destroy(st->ctx2, NULL);
    st->ctx1 = NULL;
    st->ctx2 = NULL;

    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}


const struct lys_module *
test_load_module_clb(struct ly_ctx *ctx, const char *UNUSED(name), const char *UNUSED(ns), int UNUSED(options), void *user_data)
{
    return lys_parse_mem(ctx, (char *) user_data, LYS_IN_YANG);
}

static void
test_merge_to_ctx_with_missing_schema(void **state)
{
    struct state *st = (*state);
    const char *sch_x = "module x {"
                        "  namespace urn:x;"
                        "  prefix x;"
                        "  leaf x { type string; }}";
    const char *src = "<x xmlns=\"urn:x\">x</x>";
    const char *sch_y = "module y {"
                        "  namespace urn:y;"
                        "  prefix y;"
                        "  leaf y { type string; }}";
    const char *trg = "<y xmlns=\"urn:y\">y</y>";
    const char *result = "<y xmlns=\"urn:y\">y</y><x xmlns=\"urn:x\">x</x>";
    char *printed = NULL;

    /* case 4: src contains module X schema and data, trg contains Y schema and data.
       Verify that X is loaded into Y when merging X into Y. */
    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch_x, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(st->ctx2, sch_y, LYS_IN_YANG), NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->source, NULL);

    st->target = lyd_parse_mem(st->ctx2, trg, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->target, NULL);

    ly_ctx_set_module_data_clb(st->ctx2, test_load_module_clb, (void *)sch_x);

    assert_int_equal(lyd_merge(st->target, st->source, 0), 0);

    assert_ptr_equal(st->target->schema->module->ctx, st->ctx2);
    /* check the merged data - leaf x */
    assert_ptr_not_equal(st->target->next, NULL);
    assert_ptr_equal(st->target->next->schema->module->ctx, st->ctx2);

    assert_ptr_not_equal(st->source, st->target->next);

    /* print the result after freeing the ctx1 */
    lyd_free(st->source);
    ly_ctx_destroy(st->ctx1, NULL);
    st->source = NULL;
    st->ctx1 = NULL;

    lyd_print_mem(&printed, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_merge_leafrefs(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                      "  namespace urn:x;"
                      "  prefix x;"
                      "  list l {"
                      "    key n;"
                      "    leaf n { type string; }"
                      "    leaf t { type string; }"
                      "    leaf r { type leafref { path '/l/n'; } }}}";
    const char *trg = "<l xmlns=\"urn:x\"><n>a</n></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>";
    const char *src = "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>";
    const char *res = "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>";
    char *prt = NULL;

    assert_ptr_not_equal(lys_parse_mem(st->ctx1, sch, LYS_IN_YANG), NULL);

    st->target = lyd_parse_mem(st->ctx1, trg, LYD_XML, LYD_OPT_GET);
    assert_ptr_not_equal(st->target, NULL);

    st->source = lyd_parse_mem(st->ctx1, src, LYD_XML, LYD_OPT_GET);
    assert_ptr_not_equal(st->source, NULL);

    assert_int_equal(lyd_merge(st->target, st->source, LYD_OPT_DESTRUCT), 0);
    st->source = NULL;

    lyd_print_mem(&prt, st->target, LYD_XML, LYP_WITHSIBLINGS);
    assert_string_equal(prt, res);
    free(prt);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_merge, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge2, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge3, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge4, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge5, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge_dflt1, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge_dflt2, setup_dflt, teardown_dflt),
                    cmocka_unit_test_setup_teardown(test_merge_to_trgctx1, setup_mctx, teardown_mctx),
                    cmocka_unit_test_setup_teardown(test_merge_to_trgctx2, setup_mctx, teardown_mctx),
                    cmocka_unit_test_setup_teardown(test_merge_to_ctx, setup_mctx, teardown_mctx),
                    cmocka_unit_test_setup_teardown(test_merge_to_ctx_with_missing_schema, setup_mctx, teardown_mctx),
                    cmocka_unit_test_setup_teardown(test_merge_leafrefs, setup_dflt, teardown_dflt),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
