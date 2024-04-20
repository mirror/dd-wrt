/**
 * @file test_parser_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for functions from parser_xml.c
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
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
#include "tree_data_internal.h"
#include "tree_schema.h"

static int
setup(void **state)
{
    const char *schema =
            "module a {\n"
            "    namespace urn:tests:a;\n"
            "    prefix a;\n"
            "    yang-version 1.1;\n"
            "    import ietf-yang-metadata {prefix md;}"
            "    list l1 { key \"a b c\"; leaf a {type string;} leaf b {type string;} leaf c {type int16;}"
            "        leaf d {type string;}"
            "        container cont {leaf e {type boolean;}}"
            "    }"
            "    leaf foo { type string;}\n"
            "    container c {\n"
            "        leaf x {type string;}\n"
            "        action act { input { leaf al {type string;} } output { leaf al {type uint8;} } }\n"
            "        notification n1 { leaf nl {type string;}}}\n"
            "    container cp {presence \"container switch\"; leaf y {type string;} leaf z {type int8;}}\n"
            "    anydata any {config false;}\n"
            "    anyxml anyx;\n"
            "    leaf foo2 { type string; default \"default-val\"; }\n"
            "    leaf foo3 { type uint32; }\n"
            "    notification n2;"
            "    md:annotation attr {type enumeration {enum val;}}"
            "}";

    UTEST_SETUP;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    return 0;
}

#define CHECK_PARSE_LYD(INPUT, PARSE_OPTION, VALIDATE_OPTION, TREE) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, PARSE_OPTION, VALIDATE_OPTION, LY_SUCCESS, TREE)

#define PARSER_CHECK_ERROR(INPUT, PARSE_OPTION, VALIDATE_OPTION, MODEL, RET_VAL, ERR_MESSAGE, ERR_PATH) \
    assert_int_equal(RET_VAL, lyd_parse_data_mem(UTEST_LYCTX, INPUT, LYD_XML, PARSE_OPTION, VALIDATE_OPTION, &MODEL));\
    CHECK_LOG_CTX(ERR_MESSAGE, ERR_PATH);\
    assert_null(MODEL)

#define CHECK_LYD_STRING(IN_MODEL, PRINT_OPTION, TEXT) \
    CHECK_LYD_STRING_PARAM(IN_MODEL, TEXT, LYD_XML, PRINT_OPTION)

static void
test_leaf(void **state)
{
    const char *data = "<foo xmlns=\"urn:tests:a\">foo value</foo>";
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-with-defaults", "2011-06-01", NULL));

    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "foo", 1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "foo value");

    CHECK_LYSC_NODE(tree->next->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree->next->next;
    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);
    lyd_free_all(tree);

    /* make foo2 explicit */
    data = "<foo2 xmlns=\"urn:tests:a\">default-val</foo2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_false(leaf->flags & LYD_DEFAULT);
    lyd_free_all(tree);

    /* parse foo2 but make it implicit, skip metadata xxx from missing schema */
    data = "<foo2 xmlns=\"urn:tests:a\" xmlns:wd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" "
            "wd:default=\"true\" xmlns:x=\"urn:x\" x:xxx=\"false\">default-val</foo2>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "foo2",
            1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "default-val");
    assert_true(leaf->flags & LYD_DEFAULT);
    lyd_free_all(tree);

    /* invalid value */
    data = "<l1 xmlns=\"urn:tests:a\"><a>val-a</a><b>val-b</b><c>1</c><cont><e>0</e></cont></l1>";
    PARSER_CHECK_ERROR(data, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid boolean value \"0\".",
            "Data location \"/a:l1[a='val-a'][b='val-b'][c='1']/cont/e\", line number 1.");
}

static void
test_anydata(void **state)
{
    const char *data;
    char *str;
    struct lyd_node *tree;

    data = "<any xmlns=\"urn:tests:a\">\n"
            "  <element1>\n"
            "    <x:element2 x:attr2=\"test\" xmlns:a=\"urn:tests:a\" xmlns:x=\"urn:x\">a:data</x:element2>\n"
            "  </element1>\n"
            "  <element1a/>\n"
            "</any>\n";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_R | LYS_STATUS_CURR | LYS_SET_CONFIG, 1, "any",
            1, LYS_ANYDATA, 0, 0, NULL, 0);

    const char *data_expected =
            "<any xmlns=\"urn:tests:a\">\n"
            "  <element1>\n"
            "    <element2 xmlns=\"urn:x\" xmlns:x=\"urn:x\" x:attr2=\"test\" xmlns:a=\"urn:tests:a\">a:data</element2>\n"
            "  </element1>\n"
            "  <element1a/>\n"
            "</any>\n";

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data_expected);

    assert_int_equal(LY_SUCCESS, lyd_any_value_str(tree, &str));
    lyd_free_all(tree);

    assert_int_equal(LY_SUCCESS, lyd_new_path2(NULL, UTEST_LYCTX, "/a:any", str, strlen(str), LYD_ANYDATA_XML, 0, &tree, NULL));
    free(str);
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data_expected);
    lyd_free_all(tree);
}

static void
test_anyxml(void **state)
{
    const char *data;
    char *str;
    struct lyd_node *tree;

    data = "<anyx xmlns=\"urn:tests:a\">\n"
            "  <element1>\n"
            "    <element2 x:attr2=\"test\" xmlns:x=\"urn:x\">data</element2>\n"
            "  </element1>\n"
            "  <element1a/>\n"
            "</anyx>\n";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;

    const char *data_expected =
            "<anyx xmlns=\"urn:tests:a\">\n"
            "  <element1>\n"
            "    <element2 xmlns:x=\"urn:x\" x:attr2=\"test\">data</element2>\n"
            "  </element1>\n"
            "  <element1a/>\n"
            "</anyx>\n";

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data_expected);

    assert_int_equal(LY_SUCCESS, lyd_any_value_str(tree, &str));
    lyd_free_all(tree);

    assert_int_equal(LY_SUCCESS, lyd_new_path2(NULL, UTEST_LYCTX, "/a:anyx", str, strlen(str), LYD_ANYDATA_XML, 0, &tree, NULL));
    free(str);
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data_expected);
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
    data = "<l1 xmlns=\"urn:tests:a\"><a>one</a><b>one</b><c>1</c></l1>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "l1",
            1, LYS_LIST, 0, 0, NULL, 0);
    list = (struct lyd_node_inner *)tree;
    LY_LIST_FOR(list->child, iter) {
        assert_int_not_equal(0, iter->hash);
    }
    lyd_free_all(tree);

    /* missing keys */
    PARSER_CHECK_ERROR("<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b></l1>", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"a\".", "Data location \"/a:l1[b='b'][c='1']\", line number 1.");
    CHECK_LOG_CTX("Invalid position of the key \"b\" in a list.", NULL);

    PARSER_CHECK_ERROR("<l1 xmlns=\"urn:tests:a\"><a>a</a></l1>", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"b\".", "Data location \"/a:l1[a='a']\", line number 1.");

    PARSER_CHECK_ERROR("<l1 xmlns=\"urn:tests:a\"><b>b</b><a>a</a></l1>", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"c\".", "Data location \"/a:l1[a='a'][b='b']\", line number 1.");
    CHECK_LOG_CTX("Invalid position of the key \"a\" in a list.", NULL);

    /* key duplicate */
    PARSER_CHECK_ERROR("<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b><a>a</a><c>1</c></l1>", 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Duplicate instance of \"c\".",
            "Data location \"/a:l1[a='a'][b='b'][c='1'][c='1']/c\", line number 1.");
    CHECK_LOG_CTX("Invalid position of the key \"a\" in a list.", NULL);
    CHECK_LOG_CTX("Invalid position of the key \"b\" in a list.", NULL);

    /* keys order */
    CHECK_PARSE_LYD("<l1 xmlns=\"urn:tests:a\"><d>d</d><a>a</a><c>1</c><b>b</b></l1>", 0, LYD_VALIDATE_PRESENT, tree);
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
    CHECK_LOG_CTX("Invalid position of the key \"b\" in a list.", NULL);
    lyd_free_all(tree);

    data = "<l1 xmlns=\"urn:tests:a\"><c>1</c><b>b</b><a>a</a></l1>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "l1", 1, LYS_LIST, 0, 0, NULL, 0);
    list = (struct lyd_node_inner *)tree;
    assert_non_null(leaf = (struct lyd_node_term *)list->child);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "a", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "b", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = (struct lyd_node_term *)leaf->next);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "c", 1, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LOG_CTX("Invalid position of the key \"a\" in a list.", NULL);
    CHECK_LOG_CTX("Invalid position of the key \"b\" in a list.", NULL);
    lyd_free_all(tree);

    PARSER_CHECK_ERROR(data, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid position of the key \"b\" in a list.", "Data location \"/a:l1[c='1']/b\", line number 1.");
}

static void
test_container(void **state)
{
    struct lyd_node *tree;
    struct lyd_node_inner *cont;

    CHECK_PARSE_LYD("<c xmlns=\"urn:tests:a\"/>", 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 1, LYS_CONTAINER, 0, 0, NULL, 0);
    cont = (struct lyd_node_inner *)tree;
    assert_true(cont->flags & LYD_DEFAULT);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<cp xmlns=\"urn:tests:a\"/>", 0, LYD_VALIDATE_PRESENT, tree);
    assert_non_null(tree);
    tree = tree->next;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);
    cont = (struct lyd_node_inner *)tree;
    assert_false(cont->flags & LYD_DEFAULT);
    lyd_free_all(tree);
}

static void
test_opaq(void **state)
{
    const char *data;
    struct lyd_node *tree;

    /* invalid value, no flags */
    data = "<foo3 xmlns=\"urn:tests:a\"/>";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid type uint32 empty value.", "Schema location \"/a:foo3\", line number 1.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0, LY_VALUE_XML, "foo3", 0, 0, NULL,  1,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, "<foo3 xmlns=\"urn:tests:a\"/>\n");
    lyd_free_all(tree);

    /* list, opaq flag */
    data = "<l1 xmlns=\"urn:tests:a\"/>";
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0, LY_VALUE_XML, "l1", 0, 0, NULL,  1,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, "<l1 xmlns=\"urn:tests:a\"/>\n");
    lyd_free_all(tree);

    /* missing key, no flags */
    data = "<l1 xmlns=\"urn:tests:a\">\n"
            "  <a>val_a</a>\n"
            "  <b>val_b</b>\n"
            "  <d>val_d</d>\n"
            "</l1>\n";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "List instance is missing its key \"c\".",
            "Data location \"/a:l1[a='val_a'][b='val_b']\", line number 5.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_XML, "l1", 0, 0, NULL,  1,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* invalid key, no flags */
    data = "<l1 xmlns=\"urn:tests:a\">\n"
            "  <a>val_a</a>\n"
            "  <b>val_b</b>\n"
            "  <c>val_c</c>\n"
            "</l1>\n";
    PARSER_CHECK_ERROR(data, 0, LYD_VALIDATE_PRESENT, tree, LY_EVALID,
            "Invalid type int16 value \"val_c\".",
            "Data location \"/a:l1[a='val_a'][b='val_b']/c\", line number 4.");

    /* opaq flag */
    CHECK_PARSE_LYD(data, LYD_PARSE_OPAQ | LYD_PARSE_ONLY, 0, tree);
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 0x1, LY_VALUE_XML, "l1", 0, 0, NULL,  1,  "");
    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* opaq flag and fail */
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX,
            "<a xmlns=\"ns\">\n"
            "  <b>x</b>\n"
            "  <c xmld:id=\"D\">1</c>\n"
            "</a>\n",
            LYD_XML, LYD_PARSE_OPAQ, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX("Unknown XML prefix \"xmld\".", "Data location \"/a\", line number 3.");
}

static void
test_rpc(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;
    const char *dsc = "The <edit-config> operation loads all or part of a specified\n"
            "configuration to the specified target configuration.";
    const char *ref = "RFC 6241, Section 7.2";
    const char *feats[] = {"writable-running", NULL};

    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf", "2011-06-01", feats)));

    data = "<edit-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <target>\n"
            "    <running/>\n"
            "  </target>\n"
            "  <config xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "    <l1 xmlns=\"urn:tests:a\" nc:operation=\"replace\">\n"
            "      <a>val_a</a>\n"
            "      <b>val_b</b>\n"
            "      <c>val_c</c>\n"
            "    </l1>\n"
            "    <cp xmlns=\"urn:tests:a\">\n"
            "      <z nc:operation=\"delete\"/>\n"
            "    </cp>\n"
            "  </config>\n"
            "</edit-config>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);

    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "edit-config", LYS_RPC,
            0, 0, 0, 0, 0, ref, 0);

    assert_non_null(tree);

    node = tree;
    CHECK_LYSC_ACTION((struct lysc_node_action *)node->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "edit-config", LYS_RPC,
            0, 0, 0, 0, 0, ref, 0);
    node = lyd_child(node)->next;
    dsc = "Inline Config content.";
    CHECK_LYSC_NODE(node->schema, dsc, 0, LYS_STATUS_CURR | LYS_IS_INPUT, 1, "config", 0, LYS_ANYXML, 1, 0, NULL, 0);

    node = ((struct lyd_node_any *)node)->value.tree;
    CHECK_LYSC_NODE(node->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);

    node = lyd_child(node);
    /* z has no value */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0, LY_VALUE_XML, "z", 0, 0, NULL,  1,  "");
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0x1, LY_VALUE_XML, "l1", 0, 0, NULL,  1,  "");

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<edit-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <target>\n"
            "    <running/>\n"
            "  </target>\n"
            "  <config>\n"
            "    <cp xmlns=\"urn:tests:a\">\n"
            "      <z xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"delete\"/>\n"
            "    </cp>\n"
            "    <l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\">\n"
            "      <a>val_a</a>\n"
            "      <b>val_b</b>\n"
            "      <c>val_c</c>\n"
            "    </l1>\n"
            "  </config>\n"
            "</edit-config>\n");

    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_action(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;

    data = "<c xmlns=\"urn:tests:a\">\n"
            "  <act>\n"
            "    <al>value</al>\n"
            "  </act>\n"
            "</c>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);
    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, NULL, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "act", LYS_ACTION,
            1, 0, 0, 1, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<c xmlns=\"urn:tests:a\">\n"
            "  <act>\n"
            "    <al>value</al>\n"
            "  </act>\n"
            "</c>\n");

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

    data = "<c xmlns=\"urn:tests:a\">\n"
            "  <n1>\n"
            "    <nl>value</nl>\n"
            "  </n1>\n"
            "</c>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_NOTIF_YANG, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    CHECK_LYSC_NOTIF((struct lysc_node_notif *)ntf->schema, 1, NULL, 0, 0x4, 1, 0, "n1", 1, 0, NULL, 0);

    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 1, LYS_CONTAINER, 0, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    /* top-level notif without envelope */
    data = "<n2 xmlns=\"urn:tests:a\"/>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_NOTIF_YANG, &tree, &ntf));
    ly_in_free(in, 0);

    assert_non_null(ntf);
    CHECK_LYSC_NOTIF((struct lysc_node_notif *)ntf->schema, 0, NULL, 0, 0x4, 1, 0, "n2", 0, 0, NULL, 0);

    assert_non_null(tree);
    assert_ptr_equal(ntf, tree);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
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

    data = "<c xmlns=\"urn:tests:a\">\n"
            "  <act>\n"
            "    <al>25</al>\n"
            "  </act>\n"
            "</c>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_REPLY_YANG, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);

    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, NULL, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "act", LYS_ACTION,
            1, 0, 0, 1, 0, NULL, 0);
    node = lyd_child(op);
    CHECK_LYSC_NODE(node->schema, NULL, 0, LYS_STATUS_CURR | LYS_IS_OUTPUT, 1, "al", 0, LYS_LEAF, 1, 0, NULL, 0);

    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 1, LYS_CONTAINER, 0, 0, NULL, 0);

    /* TODO print only rpc-reply node and then output subtree */
    CHECK_LYD_STRING(lyd_child(op), LYD_PRINT_WITHSIBLINGS, "<al xmlns=\"urn:tests:a\">25</al>\n");
    lyd_free_all(tree);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_netconf_rpc(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;
    const struct lyd_node *node;
    const char *dsc = "The <edit-config> operation loads all or part of a specified\n"
            "configuration to the specified target configuration.";
    const char *ref = "RFC 6241, Section 7.2";
    const char *feats[] = {"writable-running", NULL};

    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf", "2011-06-01", feats)));

    data = "<rpc message-id=\"25\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
            "<edit-config>\n"
            "  <target>\n"
            "    <running/>\n"
            "  </target>\n"
            "  <config xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "    <l1 xmlns=\"urn:tests:a\" nc:operation=\"replace\">\n"
            "      <a>val_a</a>\n"
            "      <b>val_b</b>\n"
            "      <c>val_c</c>\n"
            "    </l1>\n"
            "    <cp xmlns=\"urn:tests:a\">\n"
            "      <z nc:operation=\"delete\"/>\n"
            "    </cp>\n"
            "  </config>\n"
            "</edit-config>\n"
            "</rpc>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_NETCONF, &tree, &op));
    ly_in_free(in, 0);

    assert_non_null(op);

    node = tree;
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 1, 0, LY_VALUE_XML, "rpc", 0, 0, 0, 0, "");

    assert_non_null(tree);

    node = op;
    CHECK_LYSC_ACTION((struct lysc_node_action *)node->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "edit-config", LYS_RPC,
            0, 0, 0, 0, 0, ref, 0);
    node = lyd_child(node)->next;
    dsc = "Inline Config content.";
    CHECK_LYSC_NODE(node->schema, dsc, 0, LYS_STATUS_CURR | LYS_IS_INPUT, 1, "config", 0, LYS_ANYXML, 1, 0, NULL, 0);

    node = ((struct lyd_node_any *)node)->value.tree;
    CHECK_LYSC_NODE(node->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_PRESENCE, 1, "cp",
            1, LYS_CONTAINER, 0, 0, NULL, 0);

    node = lyd_child(node);
    /* z has no value */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0, LY_VALUE_XML, "z", 0, 0, NULL,  1,  "");
    node = node->parent->next;
    /* l1 key c has invalid value so it is at the end */
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)node, 0x1, 0x1, LY_VALUE_XML, "l1", 0, 0, NULL,  1,  "");

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"25\"/>\n");
    CHECK_LYD_STRING(op, LYD_PRINT_WITHSIBLINGS,
            "<edit-config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <target>\n"
            "    <running/>\n"
            "  </target>\n"
            "  <config>\n"
            "    <cp xmlns=\"urn:tests:a\">\n"
            "      <z xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"delete\"/>\n"
            "    </cp>\n"
            "    <l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\">\n"
            "      <a>val_a</a>\n"
            "      <b>val_b</b>\n"
            "      <c>val_c</c>\n"
            "    </l1>\n"
            "  </config>\n"
            "</edit-config>\n");

    lyd_free_all(tree);
    lyd_free_all(op);

    /* invalid anyxml nested metadata value */
    data = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"1\" pid=\"4114692032\">\n"
            "  <copy-config>\n"
            "    <target>\n"
            "      <running/>\n"
            "    </target>\n"
            "    <source>\n"
            "      <config>\n"
            "        <l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "          <a>val_a</a>\n"
            "          <b>val_b</b>\n"
            "          <c>5</c>\n"
            "          <cont nc:operation=\"merge\">\n"
            "            <e nc:operation=\"merge2\">false</e>\n"
            "          </cont>\n"
            "        </l1>\n"
            "      </config>\n"
            "    </source>\n"
            "  </copy-config>\n"
            "</rpc>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_EVALID, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_NETCONF, &tree, &op));
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Invalid enumeration value \"merge2\".",
            "Path \"/ietf-netconf:copy-config/source/config/a:l1[a='val_a'][b='val_b'][c='5']/cont/e/@ietf-netconf:operation\", line number 13.");
    lyd_free_all(tree);
    assert_null(op);
}

static void
test_netconf_action(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *op;

    data = "<rpc message-id=\"25\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
            "<action xmlns=\"urn:ietf:params:xml:ns:yang:1\">"
            "<c xmlns=\"urn:tests:a\">\n"
            "  <act>\n"
            "    <al>value</al>\n"
            "  </act>\n"
            "</c>\n"
            "</action>\n"
            "</rpc>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_NETCONF, &tree, &op));
    ly_in_free(in, 0);

    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 1, 1, LY_VALUE_XML, "rpc", 0, 0, 0, 0, "");
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)lyd_child(tree), 0, 0, LY_VALUE_XML, "action", 0, 0, 0, 0, "");

    assert_non_null(op);
    CHECK_LYSC_ACTION((struct lysc_node_action *)op->schema, NULL, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "act", LYS_ACTION,
            1, 0, 0, 1, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"25\">\n"
            "  <action xmlns=\"urn:ietf:params:xml:ns:yang:1\"/>\n"
            "</rpc>\n");
    CHECK_LYD_STRING(op, LYD_PRINT_WITHSIBLINGS,
            "<act xmlns=\"urn:tests:a\">\n"
            "  <al>value</al>\n"
            "</act>\n");

    lyd_free_all(tree);
    lyd_free_all(op);

    /* wrong namespace, element name, whatever... */
    /* TODO */
}

static void
test_netconf_reply_or_notification(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *action, *tree, *op, *op2;

    /* parse the action */
    data = "<c xmlns=\"urn:tests:a\">\n"
            "  <act>\n"
            "    <al>value</al>\n"
            "  </act>\n"
            "</c>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &action, &op));
    ly_in_free(in, 0);

    /* parse notification first */
    data = "<notification xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\">\n"
            "<eventTime>2010-12-06T08:00:01Z</eventTime>\n"
            "<c xmlns=\"urn:tests:a\">\n"
            "  <n1>\n"
            "    <nl>value</nl>\n"
            "  </n1>\n"
            "</c>\n"
            "</notification>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_NOTIF_NETCONF, &tree, &op2));
    ly_in_free(in, 0);

    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 0, 1, LY_VALUE_XML, "notification", 0, 0, 0, 0, "");
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)lyd_child(tree), 0, 0, LY_VALUE_XML, "eventTime", 0, 0, 0, 0,
            "2010-12-06T08:00:01Z");

    assert_non_null(op2);
    CHECK_LYSC_NOTIF((struct lysc_node_notif *)op2->schema, 1, NULL, 0, 0x4, 1, 0, "n1", 1, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<notification xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\">\n"
            "  <eventTime>2010-12-06T08:00:01Z</eventTime>\n"
            "</notification>\n");
    CHECK_LYD_STRING(op2, LYD_PRINT_WITHSIBLINGS,
            "<n1 xmlns=\"urn:tests:a\">\n"
            "  <nl>value</nl>\n"
            "</n1>\n");

    lyd_free_all(tree);
    lyd_free_all(op2);

    /* notification with a different order */
    data = "<notification xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\">\n"
            "<c xmlns=\"urn:tests:a\">\n"
            "  <n1>\n"
            "    <nl>value</nl>\n"
            "  </n1>\n"
            "</c>\n"
            "<eventTime>2010-12-06T08:00:01Z</eventTime>\n"
            "</notification>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_NOTIF_NETCONF, &tree, &op2));
    ly_in_free(in, 0);

    lyd_free_all(tree);
    lyd_free_all(op2);

    /* parse a data reply */
    data = "<rpc-reply message-id=\"55\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <al xmlns=\"urn:tests:a\">25</al>\n"
            "</rpc-reply>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, op, in, LYD_XML, LYD_TYPE_REPLY_NETCONF, &tree, NULL));
    ly_in_free(in, 0);

    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 1, 0, LY_VALUE_XML, "rpc-reply", 0, 0, 0, 0, "");

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS,
            "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"55\"/>\n");

    lyd_free_all(tree);
    /* it was connected to the action, do not free */

    /* parse an ok reply */
    data = "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"55\">\n"
            "  <ok/>\n"
            "</rpc-reply>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, op, in, LYD_XML, LYD_TYPE_REPLY_NETCONF, &tree, NULL));
    ly_in_free(in, 0);

    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 1, 1, LY_VALUE_XML, "rpc-reply", 0, 0, 0, 0, "");
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)lyd_child(tree), 0, 0, LY_VALUE_XML, "ok", 0, 0, 0, 0, "");

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);

    lyd_free_all(tree);

    /* parse an error reply */
    data = "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"55\">\n"
            "  <rpc-error>\n"
            "    <error-type>rpc</error-type>\n"
            "    <error-tag>missing-attribute</error-tag>\n"
            "    <error-severity>error</error-severity>\n"
            "    <error-info>\n"
            "      <bad-attribute>message-id</bad-attribute>\n"
            "      <bad-element>rpc</bad-element>\n"
            "    </error-info>\n"
            "  </rpc-error>\n"
            "</rpc-reply>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, op, in, LYD_XML, LYD_TYPE_REPLY_NETCONF, &tree, NULL));
    ly_in_free(in, 0);

    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)tree, 1, 1, LY_VALUE_XML, "rpc-reply", 0, 0, 0, 0, "");
    CHECK_LYD_NODE_OPAQ((struct lyd_node_opaq *)lyd_child(tree), 0, 1, LY_VALUE_XML, "rpc-error", 0, 0, 0, 0, "");

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);

    lyd_free_all(tree);

    lyd_free_all(action);

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

    data = "<input xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-nmda\">"
            "<datastore xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">ds:running</datastore>"
            "<config>"
            "<cp xmlns=\"urn:tests:a\"><z xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\"/></cp>"
            "<l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\">"
            "<a>val_a</a><b>val_b</b><c>val_c</c>"
            "</l1>"
            "</config></input>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, tree, in, LYD_XML, LYD_TYPE_RPC_RESTCONF, &envp, NULL));
    ly_in_free(in, 0);

    /* the same just connected to the edit-data RPC */
    data = "<edit-data xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-nmda\">"
            "<datastore xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">ds:running</datastore>"
            "<config>"
            "<cp xmlns=\"urn:tests:a\"><z xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\"/></cp>"
            "<l1 xmlns=\"urn:tests:a\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\" nc:operation=\"replace\">"
            "<a>val_a</a><b>val_b</b><c>val_c</c>"
            "</l1>"
            "</config></edit-data>";
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
    lyd_free_all(envp);
}

static void
test_restconf_reply(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree, *envp;

    assert_int_equal(LY_SUCCESS, lyd_new_path(NULL, UTEST_LYCTX, "/a:c/act", NULL, 0, &tree));

    data = "<output xmlns=\"urn:tests:a\"><al>25</al></output>";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, lyd_child(tree), in, LYD_XML, LYD_TYPE_REPLY_RESTCONF, &envp, NULL));
    ly_in_free(in, 0);

    /* connected to the RPC with the parent */
    data = "<c xmlns=\"urn:tests:a\"><act><al>25</al></act></c>";
    CHECK_LYD_STRING(tree, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
    lyd_free_all(envp);
}

static void
test_filter_attributes(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree;
    const struct lyd_node *node;
    const char *dsc;
    const char *ref = "RFC 6241, Section 7.7";
    const char *feats[] = {"writable-running", NULL};

    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf", "2011-06-01", feats)));
    assert_non_null((ly_ctx_load_module(UTEST_LYCTX, "notifications", "2008-07-14", NULL)));

    data = "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <filter type=\"xpath\" select=\"/*\"/>\n"
            "</get>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, NULL));
    ly_in_free(in, 0);
    assert_non_null(tree);

    node = tree;
    dsc = "Retrieve running configuration and device state information.";
    CHECK_LYSC_ACTION((struct lysc_node_action *)node->schema, dsc, 0, LYS_STATUS_CURR,
            1, 0, 0, 1, "get", LYS_RPC,
            1, 0, 0, 0, 0, ref, 0);
    node = lyd_child(node);
    dsc = "This parameter specifies the portion of the system\nconfiguration and state data to retrieve.";
    CHECK_LYSC_NODE(node->schema, dsc, 1, LYS_STATUS_CURR | LYS_IS_INPUT, 1, "filter", 0, LYS_ANYXML, 1, 0, NULL, 0);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);

    data = "<create-subscription xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\">\n"
            "  <filter type=\"subtree\">\n"
            "    <inner-node xmlns=\"my:urn\"/>\n"
            "  </filter>\n"
            "</create-subscription>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, NULL));
    ly_in_free(in, 0);
    assert_non_null(tree);

    CHECK_LYD_STRING(tree, LYD_PRINT_WITHSIBLINGS, data);
    lyd_free_all(tree);
}

static void
test_data_skip(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct lyd_node_term *leaf;

    /* add invalid data to a module that is not implemented */
    data = "<foo xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\"><u/></foo>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(_UC->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);

    /* add invalid data to a module that is implemented */
    data = "<fooX xmlns=\"urn:tests:a\"><u/><list><value/></list></fooX>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(_UC->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);

    /* first invalid, next valid */
    data = "<fooX xmlns=\"urn:tests:a\"><u/></fooX>  <foo xmlns=\"urn:tests:a\">foo value</foo>";
    CHECK_PARSE_LYD(data, 0, LYD_VALIDATE_PRESENT, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "foo", 1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_VALUE(leaf->value, STRING, "foo value");
    lyd_free_all(tree);
}

static void
test_metadata(void **state)
{
    const char *data;
    struct lyd_node *tree;

    /* invalid metadata value */
    data = "<c xmlns=\"urn:tests:a\" xmlns:a=\"urn:tests:a\"><x a:attr=\"value\">xval</x></c>";
    assert_int_equal(LY_EVALID, lyd_parse_data_mem(_UC->ctx, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    CHECK_LOG_CTX("Invalid enumeration value \"value\".", "Path \"/a:c/x/@a:attr\", line number 1.");
}

static void
test_subtree(void **state)
{
    const char *data;
    struct ly_in *in;
    struct lyd_node *tree;

    /* prepare data with the parent */
    data = "<l1 xmlns=\"urn:tests:a\">\n"
            "  <a>val_a</a>\n"
            "  <b>val_b</b>\n"
            "  <c>1</c>\n"
            "</l1>\n";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));

    /* parse a subtree of it */
    data = "<cont xmlns=\"urn:tests:a\">\n"
            "  <e>true</e>\n"
            "</cont>\n";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_data(UTEST_LYCTX, tree, in, LYD_XML, 0, LYD_VALIDATE_PRESENT, NULL));
    ly_in_free(in, 0);

    /* parse another container, fails */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_EVALID, lyd_parse_data(UTEST_LYCTX, tree, in, LYD_XML, 0, LYD_VALIDATE_PRESENT, NULL));
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Duplicate instance of \"cont\".", "Data location \"/a:l1[a='val_a'][b='val_b'][c='1']/cont\".");

    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_leaf, setup),
        UTEST(test_anydata, setup),
        UTEST(test_anyxml, setup),
        UTEST(test_list, setup),
        UTEST(test_container, setup),
        UTEST(test_opaq, setup),
        UTEST(test_rpc, setup),
        UTEST(test_action, setup),
        UTEST(test_notification, setup),
        UTEST(test_reply, setup),
        UTEST(test_netconf_rpc, setup),
        UTEST(test_netconf_action, setup),
        UTEST(test_netconf_reply_or_notification, setup),
        UTEST(test_restconf_rpc, setup),
        UTEST(test_restconf_reply, setup),
        UTEST(test_filter_attributes, setup),
        UTEST(test_data_skip, setup),
        UTEST(test_metadata, setup),
        UTEST(test_subtree, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
