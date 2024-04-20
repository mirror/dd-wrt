/**
 * @file test_parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for JSON parser
 *
 * Copyright (c) 2019 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "context.h"
#include "in.h"
#include "out.h"
#include "parser_data.h"
#include "printer_data.h"
#include "tests_config.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

static int
setup(void **state)
{
    const char *schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1; import ietf-yang-metadata {prefix md;}"
            "md:annotation hint { type int8;}"
            "list l1 { key \"a b c\"; leaf a {type string;} leaf b {type string;} leaf c {type int16;}"
            "  leaf d {type string;}"
            "  container cont {leaf e {type boolean;}}"
            "}"
            "leaf foo { type string;}"
            "container c {"
            "    leaf x {type string;}"
            "    action act { input { leaf al {type string;} } output { leaf al {type uint8;} } }"
            "    notification n1 { leaf nl {type string;} }"
            "}"
            "container cp {presence \"container switch\"; leaf y {type string;} leaf z {type int8;}}"
            "anydata any {config false;}"
            "anyxml axml;"
            "leaf-list ll1 { type uint8; }"
            "leaf foo2 { type string; default \"default-val\"; }"
            "leaf foo3 { type uint32; }"
            "leaf foo4 { type uint64; }"
            "rpc r1 {input {leaf l1 {type string;} leaf l2 {type string;}}}"
            "notification n2;"
            "}";

    UTEST_SETUP;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    return 0;
}

#define CHECK_PARSE_LYD(INPUT, PARSE_OPTION, VALIDATE_OPTION, TREE) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_JSON, PARSE_OPTION, VALIDATE_OPTION, LY_SUCCESS, TREE)

#define PARSER_CHECK_ERROR(INPUT, PARSE_OPTION, VALIDATE_OPTION, MODEL, RET_VAL, ERR_MESSAGE, ERR_PATH) \
    assert_int_equal(RET_VAL, lyd_parse_data_mem(UTEST_LYCTX, INPUT, LYD_JSON, PARSE_OPTION, VALIDATE_OPTION, &MODEL));\
    CHECK_LOG_CTX(ERR_MESSAGE, ERR_PATH);\
    assert_null(MODEL)

#define CHECK_LYD_STRING(IN_MODEL, PRINT_OPTION, TEXT) \
    CHECK_LYD_STRING_PARAM(IN_MODEL, TEXT, LYD_JSON, PRINT_OPTION)

static void
test_leaf(void **state)
{
    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    const char *data;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-with-defaults", "2011-06-01", NULL));

    data = "{\"a:foo\":\"foo value\"}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "foo", 1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "foo value");

    CHECK_LYSC_NODE(tree->next->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree->next->next;

    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* make foo2 explicit */
    data = "{\"a:foo2\":\"default-val\"}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_false(leaf->flags & LYD_DEFAULT);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* parse foo2 but make it implicit */
    data = "{\"a:foo2\":\"default-val\",\"@a:foo2\":{\"ietf-netconf-with-defaults:default\":true}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);

    /* print default values */
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL_TAG, data);
    lyd_free_all(tree);

    /* skip leaf */
    data = "{\"a:cp\":{\"x\":\"val\",\"y\":\"valy\",\"z\":5}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{\"a:cp\":{\"y\":\"valy\",\"z\":5}}");
    lyd_free_all(tree);

    /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:foo\":{\"a:hint\":1,\"a:hint\":2,\"x:xxx\":{\"value\":\"/x:no/x:yes\"}},\"a:foo\":\"xxx\"}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "foo", 1, LYS_LEAF, 0, 0, NULL, 0);
    CHECK_LYD_META(tree->meta, 1, "hint", 1, 1,  INT8, "1", 1);
    CHECK_LYD_META(tree->meta->next, 1, "hint", 0, 1,  INT8, "2", 2);
    assert_null(tree->meta->next->next);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS,
            "{\"a:foo\":\"xxx\",\"@a:foo\":{\"a:hint\":1,\"a:hint\":2}}");
    lyd_free_all(tree);

    PARSER_CHECK_ERROR(data, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Unknown (or not implemented) YANG module \"x\" of metadata \"x:xxx\".", "Data location \"/@a:foo\", line number 1.");

    /* missing referenced metadata node */
    PARSER_CHECK_ERROR("{\"@a:foo\" : { \"a:hint\" : 1 }}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Missing JSON data instance to be coupled with @a:foo metadata.", "Data location \"/@a:foo\", line number 1.");

    /* missing namespace for meatadata*/
    PARSER_CHECK_ERROR("{\"a:foo\" : \"value\", \"@a:foo\" : { \"hint\" : 1 }}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Metadata in JSON must be namespace-qualified, missing prefix for \"hint\".",
            "Schema location \"/a:foo\", line number 1.");

    /* invalid JSON type */
    data = "{\"a:l1\" : [{ \"a\" : \"val-a\", \"b\" : \"val-b\", \"c\" : 1, \"cont\" : { \"e\" : \"0\" } }]}";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid non-boolean-encoded boolean value \"0\".",
            "Data location \"/a:l1[a='val-a'][b='val-b'][c='1']/cont/e\", line number 1.");

    /* reverse solidus in JSON object member name */
    data = "{\"@a:foo\":{\"a:hi\\nt\":1},\"a:foo\":\"xxx\"}";
    assert_int_equal(LY_EINVAL, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Annotation definition for attribute \"a:hi\nt\" not found.", "Path \"/@a:foo/@a:hi\nt\", line number 1.");
}

static void
test_leaflist(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct lyd_node_term *ll;

    data = "{\"a:ll1\":[10,11]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(ll->value, UINT8, "10", 10);

    assert_non_null(tree->next);
    CHECK_LYSC_NODE(tree->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree->next;
    CHECK_LYD_VALUE(ll->value, UINT8, "11", 11);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* accept empty */
    data = "{\"a:ll1\":[]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_null(tree);

    /* simple metadata */
    data = "{\"a:ll1\":[10,11],\"@a:ll1\":[null,{\"a:hint\":2}]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(ll->value, UINT8, "10", 10);
    assert_null(ll->meta);

    assert_non_null(tree->next);
    CHECK_LYSC_NODE(tree->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree->next;
    CHECK_LYD_VALUE(ll->value, UINT8, "11", 11);
    CHECK_LYD_META(ll->meta, 1, "hint", 0, 1,  INT8, "2", 2);
    assert_null(ll->meta->next);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* multiple meatadata hint and unknown metadata xxx supposed to be skipped since it is from missing schema */
    data = "{\"@a:ll1\" : [{\"a:hint\" : 1, \"x:xxx\" :  { \"value\" : \"/x:no/x:yes\" }, "
            "\"a:hint\" : 10},null,{\"a:hint\" : 3}], \"a:ll1\" : [1,2,3]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(ll->value, UINT8, "1", 1);
    CHECK_LYD_META(ll->meta, 1, "hint", 1, 1,  INT8, "1", 1);
    CHECK_LYD_META(ll->meta->next, 1, "hint", 0, 1,  INT8, "10", 10);

    assert_non_null(tree->next);
    CHECK_LYSC_NODE(tree->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree->next;
    CHECK_LYD_VALUE(ll->value, UINT8, "2", 2);
    assert_null(ll->meta);

    assert_non_null(tree->next->next);
    CHECK_LYSC_NODE(tree->next->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "ll1",
            1, LYS_LEAFLIST, 0, 0, NULL, 0);
    ll = (struct lyd_node_term *)tree->next->next;
    CHECK_LYD_VALUE(ll->value, UINT8, "3", 3);
    CHECK_LYD_META(ll->meta, 1, "hint", 0, 1,  INT8, "3", 3);
    assert_null(ll->meta->next);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS,
            "{\"a:ll1\":[1,2,3],\"@a:ll1\":[{\"a:hint\":1,\"a:hint\":10},null,{\"a:hint\":3}]}");
    lyd_free_all(tree);

    /* missing referenced metadata node */
    PARSER_CHECK_ERROR("{\"@a:ll1\":[{\"a:hint\":1}]}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Missing JSON data instance to be coupled with @a:ll1 metadata.", "Data location \"/@a:ll1\", line number 1.");

    PARSER_CHECK_ERROR("{\"a:ll1\":[1],\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2}]}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Missing JSON data instance #2 of a:ll1 to be coupled with metadata.", "Schema location \"/a:ll1\", line number 1.");

    PARSER_CHECK_ERROR("{\"@a:ll1\":[{\"a:hint\":1},{\"a:hint\":2},{\"a:hint\":3}],\"a:ll1\" : [1, 2]}", 0, LYD_VALIDATE_PRESENT,
            tree, LY_EVALID, "Missing JSON data instance #3 to be coupled with @a:ll1 metadata.",
            "Data location \"/@a:ll1\", line number 1.");
}

static void
test_anydata(void **state)
{
    const char *data;
    struct lyd_node *tree;

    data = "{\"a:any\":{\"x:element1\":{\"element2\":\"/a:some/a:path\",\"list\":[{},{\"key\":\"a\"}]}}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_R | LYS_SET_CONFIG, 1, "any",
            1, LYS_ANYDATA, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:any\":{}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_R | LYS_SET_CONFIG, 1, "any",
            1, LYS_ANYDATA, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:any\":{\"node\":20}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_R | LYS_SET_CONFIG, 1, "any",
            1, LYS_ANYDATA, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
}

static void
test_anyxml(void **state)
{
    const char *data;
    struct lyd_node *tree;

    data = "{\"a:axml\":\"some-value in string\"}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":\"\"}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":55}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":false}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":null}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":[null,true,false]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":[null,true,{\"name\":[25,40, false]}]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* same as anydata tests */
    data = "{\"a:axml\":{\"x:element1\":{\"element2\":\"/a:some/a:path\",\"list\":[{},{\"key\":\"a\"}]}}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:axml\":{}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_STATUS_CURR | LYS_CONFIG_W, 1, "axml", 1, LYS_ANYXML, 0, 0, NULL, 0);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
}

static void
test_list(void **state)
{
    const char *data;
    struct lyd_node *tree, *iter;
    struct lyd_node_inner *list;
    struct lyd_node_term *leaf;

    /* check hashes */
    data = "{\"a:l1\":[{\"a\":\"one\",\"b\":\"one\",\"c\":1}]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "l1",
            1, LYS_LIST, 0, 0, NULL, 0);
    list = (struct lyd_node_inner *)tree;
    LY_LIST_FOR(list->child, iter) {
        assert_int_not_equal(0, iter->hash);
    }
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* accept empty */
    data = "{\"a:l1\":[]}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_null(tree);

    /* missing keys */
    PARSER_CHECK_ERROR("{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\"}]}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"a\".",
            "Data location \"/a:l1[b='b'][c='1']\", line number 1.");

    PARSER_CHECK_ERROR("{ \"a:l1\": [ {\"a\" : \"a\"}]}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"b\".", "Data location \"/a:l1[a='a']\", line number 1.");

    PARSER_CHECK_ERROR("{ \"a:l1\": [ {\"b\" : \"b\", \"a\" : \"a\"}]}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"c\".", "Data location \"/a:l1[a='a'][b='b']\", line number 1.");

    /* key duplicate */
    PARSER_CHECK_ERROR("{ \"a:l1\": [ {\"c\" : 1, \"b\" : \"b\", \"a\" : \"a\", \"c\" : 1}]}", 0, LYD_VALIDATE_PRESENT,
            tree, LY_EVALID, "Duplicate instance of \"c\".",
            "Data location \"/a:l1[a='a'][b='b'][c='1'][c='1']/c\", line number 1.");

    /* keys order, in contrast to XML, JSON accepts keys in any order even in strict mode */
    CHECK_PARSE_LYD("{ \"a:l1\": [ {\"d\" : \"d\", \"a\" : \"a\", \"c\" : 1, \"b\" : \"b\"}]}", 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "l1",
            1, LYS_LIST, 0, 0, NULL, 0);
    list = (struct lyd_node_inner *)tree;
    assert_non_null(leaf = (struct lyd_node_term *)list->child);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "a", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "b", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "c", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "d", 1, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LOG_CTX(NULL, NULL);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS,
            "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1,\"d\":\"d\"}]}");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("{\"a:l1\":[{\"c\":1,\"b\":\"b\",\"a\":\"a\"}]}", LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "l1",
            1, LYS_LIST, 0, 0, NULL, 0);
    list = (struct lyd_node_inner *)tree;
    assert_non_null(leaf = (struct lyd_node_term *)list->child);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "a",
            1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "b",
            1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "c",
            1, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LOG_CTX(NULL, NULL);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS,
            "{\"a:l1\":[{\"a\":\"a\",\"b\":\"b\",\"c\":1}]}");
    lyd_free_all(tree);

    /* skip unknown nested nodes */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":3,\"counters\":{\"count1\":\"c1\",\"count2\":\"c2\"}}]}";
    CHECK_PARSE_LYD(data, LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":3}]}");
    lyd_free_all(tree);

    data = "{\"a:cp\":{\"@\":{\"a:hint\":1}}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);
    CHECK_LYD_META(tree->meta, 1, "hint", 0, 1,  INT8, "1", 1);
    assert_null(tree->meta->next);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
}

static void
test_container(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct lyd_node_inner *cont;

    CHECK_PARSE_LYD("{\"a:c\":{}}", 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c",
            1, LYS_CONTAINER, 0, 0, NULL, 0);
    cont = (struct lyd_node_inner *)tree;
    assert_true(cont->flags & LYD_DEFAULT);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{}");
    lyd_free_all(tree);

    data = "{\"a:cp\":{}}";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);
    cont = (struct lyd_node_inner *)tree;
    assert_false(cont->flags & LYD_DEFAULT);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* skip container */
    CHECK_PARSE_LYD("{\"a:unknown\":{\"a\":\"val\",\"b\":5}}", 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{}");
    lyd_free_all(tree);
}

static void
test_opaq(void **state)
{
    const char *data;
    struct lyd_node *tree;

    /* invalid value, no flags */
    data = "{\"a:foo3\":[null]}";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid non-number-encoded uint32 value \"\".", "Schema location \"/a:foo3\", line number 1.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0, LY_VALUE_JSON, "foo3", 0, 0, NULL,  0,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* special chars */
    data = "{\"a:foo3\":\"ab\\\"\\\\\\r\\t\"}";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* wrong encoding */
    data = "{\"a:foo3\":\"25\"}";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0, LY_VALUE_JSON, "foo3", 0, 0, NULL,  0,  "25");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:foo4\":25}";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0, LY_VALUE_JSON, "foo4", 0, 0, NULL,  0,  "25");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* missing key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"d\":\"val_d\"}]}";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"c\".",
            "Data location \"/a:l1[a='val_a'][b='val_b']\", line number 1.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_JSON, "l1", 0, 0, NULL,  0,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* invalid key, no flags */
    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid non-number-encoded int16 value \"val_c\".",
            "Data location \"/a:l1[a='val_a'][b='val_b']/c\", line number 1.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_JSON, "l1", 0, 0, NULL,  0,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\",\"c\":{\"val\":\"val_c\"}}]}";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_JSON, "l1", 0, 0, NULL,  0,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:l1\":[{\"a\":\"val_a\",\"b\":\"val_b\"}]}";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_JSON, "l1", 0, 0, NULL,  0,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* invalid metadata */
    data = "{\"@a:foo\":\"str\",\"@a:foo3\":1,\"a:foo3\":2}";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Unknown module of node \"@a:foo\".", "Path \"/\".");
    CHECK_LOG_CTX("Missing JSON data instance to be coupled with @a:foo metadata.", "Data location \"/@a:foo\", line number 1.");

    /* empty name */
    PARSER_CHECK_ERROR("{\"@a:foo\":{\"\":0}}", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "JSON object member name cannot be a zero-length string.", "Data location \"/@a:foo\", line number 1.");

    /* opaque data tree format print */
    data =
            "{\n"
            "  \"ietf-netconf-nmda:get-data\": {\n"
            "    \"data\": {\n"
            "      \"ietf-keystore:keystore\": {\n"
            "        \"asymmetric-keys\": {\n"
            "          \"asymmetric-key\": [\n"
            "            {\n"
            "              \"name\": \"genkey\",\n"
            "              \"algorithm\": \"rsa2048\"\n"
            "            }\n"
            "          ]\n"
            "        }\n"
            "      },\n"
            "      \"ietf-netconf-server:netconf-server\": {\n"
            "        \"listen\": {\n"
            "          \"idle-timeout\": 3600,\n"
            "          \"endpoint\": [\n"
            "            {\n"
            "              \"name\": \"default-ssh\",\n"
            "              \"ssh\": {\n"
            "                \"tcp-server-parameters\": {\n"
            "                  \"local-address\": \"0.0.0.0\",\n"
            "                  \"local-port\": 830\n"
            "                },\n"
            "                \"ssh-server-parameters\": {\n"
            "                  \"server-identity\": {\n"
            "                    \"host-key\": [\n"
            "                      {\n"
            "                        \"name\": \"default-key\",\n"
            "                        \"public-key\": {\n"
            "                          \"keystore-reference\": \"genkey\"\n"
            "                        }\n"
            "                      }\n"
            "                    ]\n"
            "                  },\n"
            "                  \"client-authentication\": {\n"
            "                    \"supported-authentication-methods\": {\n"
            "                      \"publickey\": [null],\n"
            "                      \"passsword\": [null],\n"
            "                      \"other\": [\n"
            "                        \"interactive\",\n"
            "                        \"gssapi\"\n"
            "                      ]\n"
            "                    }\n"
            "                  }\n"
            "                }\n"
            "              }\n"
            "            }\n"
            "          ]\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
}

static void
test_rpc(void **state)
{
    const char *data;
    char *str;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;
    const char *dsc = "Edit data in an NMDA datastore.\n"
            "\n"
            "If an error condition occurs such that an error severity\n"
            "<rpc-error> element is generated, the server will stop\n"
            "processing the <edit-data> operation and restore the\n"
            "specified configuration to its complete state at\n"
            "the start of this <edit-data> operation.";

    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-nmda", "2019-01-07", NULL)));

    data = "{\"ietf-netconf-nmda:edit-data\":{"
            "\"datastore\":\"ietf-datastores:running\","
            "\"config\":{\"a:cp\":{\"z\":[null],\"@z\":{\"ietf-netconf:operation\":\"replace\"}},"
            "\"a:l1\":[{\"@\":{\"ietf-netconf:operation\":\"replace\"},\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}"
            "}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_RPC_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);

    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "edit-data", LYS_RPC,
            0, 0, 0, 0, 0, NULL, 0);

    node = tree;
    CHECK_LYSC_ACTION((struct lysc_node_action *)node->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "edit-data", LYS_RPC,
            0, 0, 0, 0, 0, NULL, 0);
    node = lyd_child(node)->next;
    CHECK_LYSC_NODE(node->schema, "Inline config content.", 0, LYS_STATUS_CURR | LYS_IS_INPUT, 1, "config",
            0, LYS_ANYDATA, 1, 0, NULL, 0);

    node = ((struct lyd_node_any *)node)->value.tree;
    CHECK_LYSC_NODE(node->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);
    node = lyd_child(node);
    /* z has no value */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0, LY_VALUE_JSON, "z", 0, 0, NULL,  0,  "");
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0x1, LY_VALUE_JSON, "l1", 0, 0, NULL,  0,  "");

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* append to parent */
    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, UTEST_LYCTX, "/a:r1", NULL, 0, &op));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("{\"l1\": \"some str\", \"l2\": \"some other str\"}", &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, op, in, LYD_JSON, LYD_TYPE_RPC_YANG, &tree, NULL));
    ly_in_free(in, 0);

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&str, op, LYD_JSON, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK));
    lyd_free_tree(op);
    assert_string_equal(str, "{\"a:r1\":{\"l1\":\"some str\",\"l2\":\"some other str\"}}");
    free(str);
}

static void
test_action(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;

    data = "{\"a:c\":{\"act\":{\"al\":\"value\"}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_RPC_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, NULL, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "act", LYS_ACTION,
            1, 0, 0, 1, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_notification(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *ntf;

    data = "{\"a:c\":{\"n1\":{\"nl\":\"value\"}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_YANG, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    CHECK_LYSC_NOTIF((struct lysc_node_notif *)ntf->schema, 1, NULL, 0, 0x4, 1, 0, "n1", 1, 0, NULL, 0);

    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 1, LYS_CONTAINER, 0, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "{\"a:n2\":{}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_YANG, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    CHECK_LYSC_NOTIF((struct lysc_node_notif *)ntf->schema, 0, NULL, 0, 0x4, 1, 0, "n2", 0, 0, NULL, 0);

    assert_non_null(tree);
    assert_ptr_equal(ntf, tree);

    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_reply(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;

    data = "{\"a:c\":{\"act\":{\"al\":25}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_REPLY_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, NULL, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "act", LYS_ACTION,
            1, 0, 0, 1, 0, NULL, 0);
    node = lyd_child(op);
    CHECK_LYSC_NODE(node->schema, NULL, 0, LYS_STATUS_CURR | LYS_IS_OUTPUT, 1, "al", 0, LYS_LEAF, 1, 0, NULL, 0);

    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 1, LYS_CONTAINER, 0, 0, NULL, 0);

    /* TODO print only rpc-reply node and then output subtree */
    CHECK_LYD_STRING(lyd_child(op), LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{\"a:al\":25}");
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, "{\"a:c\":{\"act\":{\"al\":25}}}");
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_restconf_rpc(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *envp;

    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-nmda", "2019-01-07", NULL)));

    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, UTEST_LYCTX, "/ietf-netconf-nmda:edit-data", NULL, 0, &tree));

    data = "{\"ietf-netconf-nmda:input\":{"
            "\"datastore\":\"ietf-datastores:running\","
            "\"config\":{\"a:cp\":{\"z\":[null],\"@z\":{\"ietf-netconf:operation\":\"replace\"}},"
            "\"a:l1\":[{\"@\":{\"ietf-netconf:operation\":\"replace\"},\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}"
            "}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, tree, in, LYD_JSON, LYD_TYPE_RPC_RESTCONF, &envp, NULL));
    ly_in_free(in, 0);

    /* the same just connected to the edit-data RPC */
    data = "{\"ietf-netconf-nmda:edit-data\":{"
            "\"datastore\":\"ietf-datastores:running\","
            "\"config\":{\"a:cp\":{\"z\":[null],\"@z\":{\"ietf-netconf:operation\":\"replace\"}},"
            "\"a:l1\":[{\"@\":{\"ietf-netconf:operation\":\"replace\"},\"a\":\"val_a\",\"b\":\"val_b\",\"c\":\"val_c\"}]}"
            "}}";
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
    lyd_free_all(envp);
}

static void
test_restconf_notification(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *ntf;

    data = "{\"ietf-restconf:notification\":{\"eventTime\":\"2013-12-21T00:01:00Z\",\"a:c\":{\"n1\":{\"nl\":\"value\"}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_RESTCONF, &tree, &ntf));
    ly_in_free(in, 0);

    /* envelopes separately */
    data = "{\"ietf-restconf:notification\":{\"eventTime\":\"2013-12-21T00:01:00Z\"}}";
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);

    /* notification with the parent node */
    data = "{\"a:c\":{\"n1\":{\"nl\":\"value\"}}}";
    CHECK_LYD_STRING(lyd_parent(ntf), LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);

    lyd_free_all(tree);
    lyd_free_all(ntf);

    /* wrong order */
    data = "{\"ietf-restconf:notification\":{\"a:n2\":{},\"eventTime\":\"2013-12-21T00:01:00Z\"}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_RESTCONF, &tree, &ntf));
    ly_in_free(in, 0);

    lyd_free_all(tree);
    lyd_free_all(ntf);

    /* unknown notification */
    data = "{\"ietf-restconf:notification\":{\"eventTime\":\"2013-12-21T00:01:00Z\",\"invalid:n2\":{}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_EVALID, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_RESTCONF, &tree, &ntf));
    UTEST_LOG_CTX_CLEAN;
    ly_in_free(in, 0);
    lyd_free_all(tree);
}

static void
test_restconf_reply(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *envp;

    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, UTEST_LYCTX, "/a:c/act", NULL, 0, &tree));

    data = "{\"a:output\":{\"al\":25}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, lyd_child(tree), in, LYD_JSON, LYD_TYPE_REPLY_RESTCONF, &envp, NULL));
    ly_in_free(in, 0);

    /* connected to the RPC with the parent */
    data = "{\"a:c\":{\"act\":{\"al\":25}}}";
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
    lyd_free_all(envp);
}

static void
test_metadata(void **state)
{
    const char *data;
    struct lyd_node *tree;

    /* invalid metadata value */
    data = "{\"a:c\":{\"x\":\"xval\",\"@x\":{\"a:hint\":\"value\"}}}";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(_UC->ctx, data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    CHECK_LOG_CTX("Invalid non-number-encoded int8 value \"value\".", "Path \"/a:c/x/@a:hint\", line number 1.");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_leaf, setup),
        UTEST(test_leaflist, setup),
        UTEST(test_anydata, setup),
        UTEST(test_anyxml, setup),
        UTEST(test_list, setup),
        UTEST(test_container, setup),
        UTEST(test_opaq, setup),
        UTEST(test_rpc, setup),
        UTEST(test_action, setup),
        UTEST(test_notification, setup),
        UTEST(test_reply, setup),
        UTEST(test_restconf_rpc, setup),
        UTEST(test_restconf_notification, setup),
        UTEST(test_restconf_reply, setup),
        UTEST(test_metadata, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
