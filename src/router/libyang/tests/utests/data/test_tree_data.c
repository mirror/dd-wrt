/**
 * @file test_tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from tree_data.c
 *
 * Copyright (c) 2018-2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"
#include "ly_common.h"
#include "path.h"
#include "xpath.h"

static int
setup(void **state)
{
    const char *schema1 = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;"
            "revision 2014-05-08;"
            "leaf bar {type string;}"
            "list l1 { key \"a b\"; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "leaf foo { type string;}"
            "leaf-list ll { type string;}"
            "container c {leaf-list x {type string;}}"
            "anydata any {config false;}"
            "list l2 {config false;"
            "    container c{leaf x {type string;} leaf-list d {type string;}}"
            "}}";

    const char *schema2 = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "list l2 {config false;"
            "    container c{leaf x {type string;}}}"
            "anydata any {config false;}"
            "}";

    const char *schema3 = "module c {yang-version 1.1; namespace \"http://example.com/main\";prefix m;"
            "import \"ietf-inet-types\" {prefix inet;}"
            "typedef optional-ip-address {type union {"
            "   type inet:ip-address;"
            "   type string;"
            "}}"
            "container cont {"
            "   list nexthop {min-elements 1; key \"gateway\";"
            "       leaf gateway {type optional-ip-address;}"
            "   }"
            "   leaf-list pref {type inet:ipv6-prefix;}"
            "}}";

    UTEST_SETUP;

    UTEST_ADD_MODULE(schema1, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(schema2, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(schema3, LYS_IN_YANG, NULL, NULL);

    return 0;
}

#define CHECK_PARSE_LYD(INPUT, PARSE_OPTION, VALIDATE_OPTION, TREE) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, PARSE_OPTION, VALIDATE_OPTION, LY_SUCCESS, TREE)

#define CHECK_PARSE_LYD_PARAM_CTX(CTX, INPUT, PARSE_OPTION, OUT_NODE) \
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(CTX, INPUT, LYD_XML, PARSE_OPTION, LYD_VALIDATE_PRESENT, &OUT_NODE)); \
    assert_non_null(OUT_NODE);

#define RECREATE_CTX_WITH_MODULE(CTX, MODULE) \
    ly_ctx_destroy(CTX); \
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &CTX)); \
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(MODULE, &_UC->in)); \
    assert_int_equal(LY_SUCCESS, lys_parse(CTX, _UC->in, LYS_IN_YANG, NULL, NULL)); \
    ly_in_free(_UC->in, 0);

static void
test_compare(void **state)
{
    struct lyd_node *tree1, *tree2;
    const char *data1;
    const char *data2;

    assert_int_equal(LY_SUCCESS, lyd_compare_single(NULL, NULL, 0));

    data1 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    data2 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>y</c></l1>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    assert_int_equal(LY_ENOT, lyd_compare_single(((struct lyd_node_inner *)tree1)->child, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1->next, tree2->next, LYD_COMPARE_FULL_RECURSION));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next->next, tree2->next, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<ll xmlns=\"urn:tests:a\">a</ll><ll xmlns=\"urn:tests:a\">b</ll>";
    data2 = "<ll xmlns=\"urn:tests:a\">b</ll>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(NULL, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, NULL, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>y</x></c>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>x</x><x>y</x></c>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x></any>";
    data2 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1->next, tree2->next, 0));
    lyd_free_all(tree1);
    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>c</x><x>a</x><x>b</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>a</x><x>b</x><x>c</x></c>";
    CHECK_PARSE_LYD(data1, 0, LYD_VALIDATE_PRESENT, tree1);
    CHECK_PARSE_LYD(data2, 0, LYD_VALIDATE_PRESENT, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);
}

static void
test_compare_diff_ctx(void **state)
{
    struct lyd_node *tree1, *tree2;
    const char *data1, *data2;
    struct ly_ctx *ctx2 = NULL;
    const char *module;

    /* create second context with the same schema */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "list l2 {config false;"
            "    container c{leaf x {type string;}}"
            "}}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, 0, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, 0, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with schema that has a different name */
    module = "module c {namespace urn:tests:c;prefix c;yang-version 1.1;"
            "revision 2014-05-08;"
            "list l2 {config false;"
            "    container c{leaf x {type string;}}"
            "}}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:c\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, 0, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, 0, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with schema that has a different revision */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2015-05-08;"
            "list l2 {config false;"
            "    container c{leaf x {type string;}}"
            "}}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, 0, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, 0, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with schema that has no revision */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "list l2 {config false;"
            "    container c{leaf x {type string;}}"
            "}}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, 0, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, 0, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with schema that has a different parent nodetype */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "container l2 {config false;"
            "    container c{leaf x {type string;}}"
            "}}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:b\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, 0, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, 0, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(lyd_child(lyd_child(tree1)), lyd_child(lyd_child(tree2)), 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with the same opaq data nodes */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "anydata any {config false;}"
            "}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<any xmlns=\"urn:tests:b\" xmlns:aa=\"urn:tests:b\"><x>aa:x</x></any>";
    data2 = "<any xmlns=\"urn:tests:b\" xmlns:bb=\"urn:tests:b\"><x>bb:x</x></any>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, LYD_PARSE_ONLY, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, LYD_PARSE_ONLY, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with the different opaq data node value */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "anydata any {config false;}"
            "}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<any xmlns=\"urn:tests:b\" xmlns:aa=\"urn:tests:b\"><x>aa:x</x></any>";
    data2 = "<any xmlns=\"urn:tests:b\" xmlns:bb=\"urn:tests:b\"><x>bb:y</x></any>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, LYD_PARSE_ONLY, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, LYD_PARSE_ONLY, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* recreate second context with the wrong prefix in opaq data node value */
    module = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;"
            "revision 2014-05-08;"
            "anydata any {config false;}"
            "}";
    RECREATE_CTX_WITH_MODULE(ctx2, module);
    data1 = "<any xmlns=\"urn:tests:b\" xmlns:aa=\"urn:tests:b\"><x>aa:x</x></any>";
    data2 = "<any xmlns=\"urn:tests:b\" xmlns:bb=\"urn:tests:b\"><x>cc:x</x></any>";
    CHECK_PARSE_LYD_PARAM_CTX(UTEST_LYCTX, data1, LYD_PARSE_ONLY, tree1);
    CHECK_PARSE_LYD_PARAM_CTX(ctx2, data2, LYD_PARSE_ONLY, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* clean up */
    ly_ctx_destroy(ctx2);
    _UC->in = NULL;
}

static void
test_dup(void **state)
{
    struct lyd_node *tree1, *tree2;
    const char *result;
    const char *data;

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, LYD_DUP_RECURSIVE, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    result = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b></l1>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    lyd_free_all(tree1);
    CHECK_PARSE_LYD(result, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    result = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_siblings(tree1, NULL, LYD_DUP_RECURSIVE, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2->next, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree2);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1->next, NULL, LYD_DUP_RECURSIVE, &tree2));
    lyd_free_all(tree1);
    CHECK_PARSE_LYD(result, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree2);

    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1->next, NULL, 0, &tree2));
    lyd_free_all(tree1);
    result = "<l2 xmlns=\"urn:tests:a\"/>";
    CHECK_PARSE_LYD_PARAM(result, LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<any xmlns=\"urn:tests:a\"><c><a>a</a></c></any>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(lyd_child(lyd_child(tree1->next)), NULL, LYD_DUP_WITH_PARENTS, &tree2));
    int unsigned flag = LYS_CONFIG_R | LYS_SET_ENUM;

    CHECK_LYSC_NODE(tree2->schema, NULL, 0, flag, 1, "x", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, (struct lyd_node *)tree2->parent->parent,
            LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(((struct lyd_node_inner *)tree1)->child->prev, NULL,
            LYD_DUP_WITH_PARENTS, &tree2));
    flag = LYS_CONFIG_W | LYS_SET_ENUM;
    CHECK_LYSC_NODE(tree2->schema, NULL, 0, flag, 1, "c", 0, LYS_LEAF, 1, 0, NULL, 0);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, (struct lyd_node *)tree2->parent, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1->next, NULL, 0, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_dup_single(lyd_child(lyd_child(tree1->next)), (struct lyd_node_inner *)tree2,
            LYD_DUP_WITH_PARENTS, NULL));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2, LYD_COMPARE_FULL_RECURSION));
    lyd_free_all(tree1);
    lyd_free_all(tree2);

    /* invalid */
    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree1);
    assert_int_equal(LY_EINVAL, lyd_dup_single(((struct lyd_node_inner *)tree1)->child->prev,
            (struct lyd_node_inner *)tree1->next, LYD_DUP_WITH_PARENTS, NULL));
    CHECK_LOG_CTX("None of the duplicated node \"c\" schema parents match the provided parent \"c\".", NULL, 0);
    lyd_free_all(tree1);
}

static void
test_target(void **state)
{
    const struct lyd_node_term *term;
    struct lyd_node *tree;
    struct lyxp_expr *exp;
    struct ly_path *path;
    const char *path_str = "/a:l2[2]/c/d[3]";
    const char *data =
            "<l2 xmlns=\"urn:tests:a\"><c>"
            "  <d>a</d>"
            "  </c></l2>"
            "<l2 xmlns=\"urn:tests:a\"><c>"
            "  <d>a</d>"
            "  <d>b</d>"
            "  <d>b</d>"
            "  <d>c</d>"
            "</c></l2>"
            "<l2 xmlns=\"urn:tests:a\"><c>"
            "</c></l2>";

    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_int_equal(LY_SUCCESS, ly_path_parse(UTEST_LYCTX, NULL, path_str, strlen(path_str), 0, LY_PATH_BEGIN_EITHER,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_SIMPLE, &exp));
    assert_int_equal(LY_SUCCESS, ly_path_compile(UTEST_LYCTX, NULL, NULL, NULL, exp, LY_PATH_OPER_INPUT,
            LY_PATH_TARGET_SINGLE, 1, LY_VALUE_JSON, NULL, &path));
    assert_int_equal(LY_SUCCESS, lyd_find_target(path, tree, (struct lyd_node **)&term));

    const int unsigned flag = LYS_CONFIG_R | LYS_SET_ENUM | LYS_ORDBY_USER;

    CHECK_LYSC_NODE(term->schema, NULL, 0, flag, 1, "d", 0, LYS_LEAFLIST, 1, 0, NULL, 0);
    assert_string_equal(lyd_get_value(&term->node), "b");
    assert_string_equal(lyd_get_value(term->prev), "b");

    lyd_free_all(tree);
    ly_path_free(UTEST_LYCTX, path);
    lyxp_expr_free(UTEST_LYCTX, exp);
}

static void
test_list_pos(void **state)
{
    const char *data;
    struct lyd_node *tree;

    data = "<bar xmlns=\"urn:tests:a\">test</bar>"
            "<l1 xmlns=\"urn:tests:a\"><a>one</a><b>one</b></l1>"
            "<l1 xmlns=\"urn:tests:a\"><a>two</a><b>two</b></l1>"
            "<foo xmlns=\"urn:tests:a\">test</foo>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(0, lyd_list_pos(tree));
    assert_int_equal(1, lyd_list_pos(tree->next));
    assert_int_equal(2, lyd_list_pos(tree->next->next));
    assert_int_equal(0, lyd_list_pos(tree->next->next->next));
    lyd_free_all(tree);

    data = "<ll xmlns=\"urn:tests:a\">one</ll>"
            "<ll xmlns=\"urn:tests:a\">two</ll>"
            "<ll xmlns=\"urn:tests:a\">three</ll>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_int_equal(1, lyd_list_pos(tree));
    assert_int_equal(2, lyd_list_pos(tree->next));
    assert_int_equal(3, lyd_list_pos(tree->next->next));
    lyd_free_all(tree);

    data = "<ll xmlns=\"urn:tests:a\">one</ll>"
            "<l1 xmlns=\"urn:tests:a\"><a>one</a><b>one</b></l1>"
            "<ll xmlns=\"urn:tests:a\">two</ll>"
            "<l1 xmlns=\"urn:tests:a\"><a>two</a><b>two</b></l1>"
            "<ll xmlns=\"urn:tests:a\">three</ll>"
            "<l1 xmlns=\"urn:tests:a\"><a>three</a><b>three</b></l1>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_string_equal("l1", tree->schema->name);
    assert_int_equal(1, lyd_list_pos(tree));
    assert_int_equal(2, lyd_list_pos(tree->next));
    assert_int_equal(3, lyd_list_pos(tree->next->next));
    assert_string_equal("ll", tree->next->next->next->schema->name);
    assert_int_equal(1, lyd_list_pos(tree->next->next->next));
    assert_int_equal(2, lyd_list_pos(tree->next->next->next->next));
    assert_int_equal(3, lyd_list_pos(tree->next->next->next->next->next));
    lyd_free_all(tree);
}

static void
test_first_sibling(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct lyd_node_inner *parent;

    data = "<bar xmlns=\"urn:tests:a\">test</bar>"
            "<l1 xmlns=\"urn:tests:a\"><a>one</a><b>one</b><c>one</c></l1>"
            "<foo xmlns=\"urn:tests:a\">test</foo>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_ptr_equal(tree, lyd_first_sibling(tree->next));
    assert_ptr_equal(tree, lyd_first_sibling(tree));
    assert_ptr_equal(tree, lyd_first_sibling(tree->prev));
    parent = (struct lyd_node_inner *)tree->next;
    assert_int_equal(LYS_LIST, parent->schema->nodetype);
    assert_ptr_equal(parent->child, lyd_first_sibling(parent->child->next));
    assert_ptr_equal(parent->child, lyd_first_sibling(parent->child));
    assert_ptr_equal(parent->child, lyd_first_sibling(parent->child->prev));
    lyd_free_all(tree);
}

static void
test_find_path(void **state)
{
    struct lyd_node *root;
    const struct lys_module *mod;

    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "c");
    assert_non_null(mod);

    assert_int_equal(LY_SUCCESS, lyd_new_inner(NULL, mod, "cont", 0, &root));
    assert_int_equal(LY_SUCCESS, lyd_new_path(root, NULL, "/c:cont/nexthop[gateway='10.0.0.1']", NULL, LYD_NEW_PATH_UPDATE, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_path(root, NULL, "/c:cont/nexthop[gateway='2100::1']", NULL, LYD_NEW_PATH_UPDATE, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_path(root, NULL, "/c:cont/pref[.='fc00::/64']", NULL, 0, NULL));

    assert_int_equal(LY_SUCCESS, lyd_find_path(root, "/c:cont/nexthop[gateway='10.0.0.1']", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_find_path(root, "/c:cont/nexthop[gateway='2100::1']", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_find_path(root, "/c:cont/pref[.='fc00::/64']", 0, NULL));

    assert_int_equal(LY_EVALID, lyd_find_path(root, "/cont", 0, NULL));
    CHECK_LOG_CTX("Prefix missing for \"cont\" in path.", "/c:cont", 0);
    assert_int_equal(LY_SUCCESS, lyd_find_path(root, "nexthop[gateway='2100::1']", 0, NULL));

    lyd_free_all(root);
}

static void
test_data_hash(void **state)
{
    struct lyd_node *tree;
    const char *schema, *data;

    schema =
            "module test-data-hash {"
            "  yang-version 1.1;"
            "  namespace \"urn:tests:tdh\";"
            "  prefix t;"
            "  container c {"
            "    leaf-list ll {"
            "      type string;"
            "    }"
            "  }"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* The number of <ll/> must be greater or equal to LYD_HT_MIN_ITEMS
     * for the correct test run. It should guarantee the creation of a hash table.
     */
    assert_true(LYD_HT_MIN_ITEMS <= 4);
    data =
            "<c xmlns='urn:tests:tdh'>"
            "  <ll/>"
            "  <ll/>"
            "  <ll/>"
            "  <ll/>"
            "</c>";

    /* The run must not crash due to the assert that checks the hash. */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"ll\".", "/test-data-hash:c/ll[.='']", 1);
    lyd_free_all(tree);
}

static void
test_lyxp_vars(void **UNUSED(state))
{
    struct lyxp_var *vars;

    /* Test free. */
    vars = NULL;
    lyxp_vars_free(vars);

    /* Bad arguments for lyxp_vars_add(). */
    assert_int_equal(LY_EINVAL, lyxp_vars_set(NULL, "var1", "val1"));
    assert_int_equal(LY_EINVAL, lyxp_vars_set(&vars, NULL, "val1"));
    assert_int_equal(LY_EINVAL, lyxp_vars_set(&vars, "var1", NULL));
    lyxp_vars_free(vars);
    vars = NULL;

    /* Add one item. */
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "val1"));
    assert_int_equal(LY_ARRAY_COUNT(vars), 1);
    assert_string_equal(vars[0].name, "var1");
    assert_string_equal(vars[0].value, "val1");
    lyxp_vars_free(vars);
    vars = NULL;

    /* Add three items. */
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "val1"));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var2", "val2"));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var3", "val3"));
    assert_int_equal(LY_ARRAY_COUNT(vars), 3);
    assert_string_equal(vars[0].name, "var1");
    assert_string_equal(vars[0].value, "val1");
    assert_string_equal(vars[1].name, "var2");
    assert_string_equal(vars[1].value, "val2");
    assert_string_equal(vars[2].name, "var3");
    assert_string_equal(vars[2].value, "val3");
    lyxp_vars_free(vars);
    vars = NULL;

    /* Change value of a variable. */
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "val1"));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var2", "val2"));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "new_value"));
    assert_string_equal(vars[0].name, "var1");
    assert_string_equal(vars[0].value, "new_value");
    assert_string_equal(vars[1].name, "var2");
    assert_string_equal(vars[1].value, "val2");
    lyxp_vars_free(vars);
    vars = NULL;
}

static void
test_data_leafref_nodes(void **state)
{
    struct lyd_node *tree, *iter;
    struct lyd_node_term *target_node = NULL, *leafref_node;
    const struct lyd_leafref_links_rec *rec;
    const char *schema, *data, *value;

    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_LEAFREF_LINKING);

    schema =
            "module test-data-hash {"
            "  yang-version 1.1;"
            "  namespace \"urn:tests:tdh\";"
            "  prefix t;"
            "  leaf-list ll {"
            "    type string;"
            "  }"
            "  container c1 {"
            "    leaf ref1 {"
            "      type leafref {"
            "        path \"../../ll\";"
            "      }"
            "    }"
            "  }"
            "  leaf ref2 {"
            "    type leafref {"
            "      path \"../ll\";"
            "    }"
            "  }"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    data =
            "{"
            "  \"test-data-hash:ll\": [\"qwe\", \"asd\"],"
            "  \"test-data-hash:c1\": { \"ref1\": \"qwe\"},"
            "  \"test-data-hash:ref2\": \"asd\""
            "}";

    /* The run must not crash due to the assert that checks the hash. */
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    LY_LIST_FOR(tree, iter) {
        if (strcmp(iter->schema->name, "ll") == 0) {
            value = lyd_get_value(iter);
            if (strcmp(value, "asd") == 0) {
                target_node = (struct lyd_node_term *)iter;
            }
        }
        if (strcmp(iter->schema->name, "ref2") == 0) {
            leafref_node = (struct lyd_node_term *)iter;
        }
    }

    /* verify state after leafref plugin validation */
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->target_nodes));
    assert_ptr_equal(rec->target_nodes[0], target_node);
    /* value modification of target */
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)target_node, "ASD"));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(leafref_node, &rec));
    /* change back to original value */
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)target_node, "asd"));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(leafref_node, &rec));
    /* linking the whole tree again */
    assert_int_equal(LY_SUCCESS, lyd_leafref_link_node_tree(tree));
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->target_nodes));
    assert_ptr_equal(rec->target_nodes[0], target_node);
    /* value modification of leafref */
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)leafref_node, "qwe"));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)leafref_node, "asd"));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(leafref_node, &rec));
    /* linking the whole tree again */
    assert_int_equal(LY_SUCCESS, lyd_leafref_link_node_tree(tree));
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->target_nodes));
    assert_ptr_equal(rec->target_nodes[0], target_node);
    /* freeing whole tree */
    lyd_free_all(tree);
}

static void
test_data_leafref_nodes2(void **state)
{
    struct lyd_node *tree, *iter;
    const char *schema, *data;
    struct lyd_node_term *leafref_node = NULL;
    const struct lyd_node_term *target_node1, *target_node2;
    const struct lyd_leafref_links_rec *rec;

    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_LEAFREF_LINKING);

    schema =
            "module test-data-hash {"
            "  yang-version 1.1;"
            "  namespace \"urn:tests:tdh\";"
            "  prefix t;"
            "  list l1 {"
            "    key \"l1 l2\";"
            "    leaf l1 {"
            "      type string;"
            "    }"
            "    leaf l2 {"
            "      type string;"
            "    }"
            "  }"
            "  leaf ref1 {"
            "    type leafref {"
            "      path \"../l1/l1\";"
            "    }"
            "  }"
            "  leaf-list ll1 {"
            "    type string;"
            "    config false;"
            "  }"
            "  leaf ref2 {"
            "    type leafref {"
            "      path \"../ll1\";"
            "    }"
            "    config false;"
            "  }"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    data =
            "{"
            "  \"test-data-hash:l1\": ["
            "    {\"l1\": \"A\", \"l2\": \"B\"},"
            "    {\"l1\": \"A\", \"l2\": \"C\"}"
            "  ],"
            "  \"test-data-hash:ref1\": \"A\","
            "  \"test-data-hash:ll1\": [\"asd\", \"qwe\", \"asd\"],"
            "  \"test-data-hash:ref2\": \"asd\""
            "}";

    /* The run must not crash due to the assert that checks the hash. */
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    LY_LIST_FOR(tree, iter) {
        if (strcmp(iter->schema->name, "ref1") == 0) {
            leafref_node = (struct lyd_node_term *)iter;
        }
    }

    /* verify state after leafref plugin validation */
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(2, LY_ARRAY_COUNT(rec->target_nodes));
    target_node1 = rec->target_nodes[0];
    target_node2 = rec->target_nodes[1];
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node1, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node2, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    /* value modification of leafref to remove all links*/
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)leafref_node, "qwe"));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node1, &rec));
    assert_int_equal(LY_ENOTFOUND, lyd_leafref_get_links(target_node2, &rec));
    /* linking the whole tree again */
    assert_int_equal(LY_SUCCESS, lyd_change_term((struct lyd_node *)leafref_node, "A"));
    assert_int_equal(LY_SUCCESS, lyd_leafref_link_node_tree(tree));
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(2, LY_ARRAY_COUNT(rec->target_nodes));
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node1, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node2, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);

    /* verify duplicated value in leaf-list */
    LY_LIST_FOR(tree, iter) {
        if (strcmp(iter->schema->name, "ref2") == 0) {
            leafref_node = (struct lyd_node_term *)iter;
        }
    }

    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(leafref_node, &rec));
    assert_int_equal(2, LY_ARRAY_COUNT(rec->target_nodes));
    target_node1 = rec->target_nodes[0];
    target_node2 = rec->target_nodes[1];
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node1, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    assert_int_equal(LY_SUCCESS, lyd_leafref_get_links(target_node2, &rec));
    assert_int_equal(1, LY_ARRAY_COUNT(rec->leafref_nodes));
    assert_ptr_equal(rec->leafref_nodes[0], leafref_node);
    /* freeing whole tree */
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_compare, setup),
        UTEST(test_compare_diff_ctx, setup),
        UTEST(test_dup, setup),
        UTEST(test_target, setup),
        UTEST(test_list_pos, setup),
        UTEST(test_first_sibling, setup),
        UTEST(test_find_path, setup),
        UTEST(test_data_hash, setup),
        UTEST(test_lyxp_vars),
        UTEST(test_data_leafref_nodes),
        UTEST(test_data_leafref_nodes2),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
