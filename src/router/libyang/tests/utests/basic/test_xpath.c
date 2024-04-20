/**
 * @file test_xpath.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for XPath evaluation
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <string.h>

#include "context.h"
#include "parser_data.h"
#include "set.h"
#include "tests_config.h"
#include "tree_data.h"
#include "tree_schema.h"

const char *schema_a =
        "module a {\n"
        "    namespace urn:tests:a;\n"
        "    prefix a;\n"
        "    yang-version 1.1;\n"
        "\n"
        "    identity id_a;\n"
        "    identity id_b {\n"
        "        base id_a;\n"
        "    }\n"
        "    identity id_c {\n"
        "        base id_b;\n"
        "    }\n"
        "\n"
        "    list l1 {\n"
        "        key \"a b\";\n"
        "        leaf a {\n"
        "            type string;\n"
        "        }\n"
        "        leaf b {\n"
        "            type string;\n"
        "        }\n"
        "        leaf c {\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        "    leaf foo {\n"
        "        type string;\n"
        "    }\n"
        "    leaf foo2 {\n"
        "        type uint8;\n"
        "    }\n"
        "    leaf foo3 {\n"
        "        type identityref {\n"
        "            base id_a;\n"
        "        }\n"
        "    }\n"
        "    leaf foo4 {\n"
        "        type decimal64 {\n"
        "            fraction-digits 5;\n"
        "        }\n"
        "    }\n"
        "    container c {\n"
        "        leaf x {\n"
        "            type string;\n"
        "        }\n"
        "        list ll {\n"
        "            key \"a\";\n"
        "            leaf a {\n"
        "                type string;\n"
        "            }\n"
        "            list ll {\n"
        "                key \"a\";\n"
        "                leaf a {\n"
        "                    type string;\n"
        "                }\n"
        "                leaf b {\n"
        "                    type string;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "        leaf-list ll2 {\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    rpc r {\n"
        "        input {\n"
        "            leaf l {\n"
        "                type string;\n"
        "            }\n"
        "        }\n"
        "        output {\n"
        "            leaf l {\n"
        "                type string;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}";

static int
setup(void **state)
{
    UTEST_SETUP;

    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, NULL);
    lys_parse_path(UTEST_LYCTX, TESTS_DIR_MODULES_YANG "/ietf-interfaces@2014-05-08.yang", LYS_IN_YANG, NULL);

    return 0;
}

static void
test_predicate(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct ly_set *set;

    data =
            "<foo2 xmlns=\"urn:tests:a\">50</foo2>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a1</a>"
            "  <b>b1</b>"
            "  <c>c1</c>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a2</a>"
            "  <b>b2</b>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a3</a>"
            "  <b>b3</b>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a4</a>"
            "  <b>b4</b>"
            "  <c>c4</c>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a5</a>"
            "  <b>b5</b>"
            "  <c>c5</c>"
            "</l1>"
            "<c xmlns=\"urn:tests:a\">"
            "  <x>key2</x>"
            "  <ll>"
            "    <a>key1</a>"
            "    <ll>"
            "      <a>key11</a>"
            "      <b>val11</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key12</a>"
            "      <b>val12</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key13</a>"
            "      <b>val13</b>"
            "    </ll>"
            "  </ll>"
            "  <ll>"
            "    <a>key2</a>"
            "    <ll>"
            "      <a>key21</a>"
            "      <b>val21</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key22</a>"
            "      <b>val22</b>"
            "    </ll>"
            "  </ll>"
            "  <ll>"
            "    <a>key3</a>"
            "    <ll>"
            "      <a>key31</a>"
            "      <b>val31</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key32</a>"
            "      <b>val32</b>"
            "    </ll>"
            "  </ll>"
            "</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* predicate after number */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/foo2[4[3 = 3]]", &set));
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    /* reverse axis */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/child::ll[2]/preceding::ll[3]", &set));
    assert_int_equal(1, set->count);
    assert_string_equal("key11", lyd_get_value(lyd_child(set->dnodes[0])));
    ly_set_free(set, NULL);

    /* special predicate evaluated using hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:l1[a=concat('a', '1')][b=substring('ab1',2)]", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a=../x]", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* cannot use hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a=substring(ll/a,1,4)]", &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* nested predicate */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/a:ll[a:a=string(/a:l1[a:a='foo']/a:a)]/a:a", &set));
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_union(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct ly_set *set;

    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* Predicate for operand. */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/l1[c[../a = 'a1'] | c]/a", &set));
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_invalid(void **state)
{
    const char *data =
            "<foo2 xmlns=\"urn:tests:a\">50</foo2>";
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_EVALID, lyd_find_xpath(tree, "/a:foo2[.=]", &set));
    assert_null(set);
    CHECK_LOG_CTX("Unexpected XPath token \"]\" (\"]\").", NULL, 0);

    assert_int_equal(LY_EVALID, lyd_find_xpath(tree, "/a:", &set));
    assert_null(set);
    CHECK_LOG_CTX("Invalid character 'a'[2] of expression '/a:'.", NULL, 0);

    lyd_free_all(tree);
}

static void
test_hash(void **state)
{
    const char *data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>\n"
            "<foo xmlns=\"urn:tests:a\">foo value</foo>\n"
            "<c xmlns=\"urn:tests:a\">\n"
            "    <x>val</x>\n"
            "    <ll>\n"
            "        <a>val_a</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "            <b>val</b>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll>\n"
            "        <a>val_b</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "            <b>val</b>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll>\n"
            "        <a>val_c</a>\n"
            "        <ll>\n"
            "            <a>val_a</a>\n"
            "        </ll>\n"
            "        <ll>\n"
            "            <a>val_b</a>\n"
            "        </ll>\n"
            "    </ll>\n"
            "    <ll2>one</ll2>\n"
            "    <ll2>two</ll2>\n"
            "    <ll2>three</ll2>\n"
            "    <ll2>four</ll2>\n"
            "</c>";
    struct lyd_node *tree, *node;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* top-level, so hash table is not ultimately used but instances can be compared based on hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:l1[a='a3'][b='b3']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "l1");
    node = lyd_child(node);
    assert_string_equal(node->schema->name, "a");
    assert_string_equal(lyd_get_value(node), "a3");

    ly_set_free(set, NULL);

    /* hashes should be used for both searches (well, there are not enough nested ll instances, so technically not true) */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a='val_b']/ll[a='val_b']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "ll");
    node = lyd_child(node);
    assert_string_equal(node->schema->name, "a");
    assert_string_equal(lyd_get_value(node), "val_b");
    node = node->next;
    assert_string_equal(node->schema->name, "b");
    assert_null(node->next);

    ly_set_free(set, NULL);

    /* hashes are not used */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c//ll[a='val_b']", &set));
    assert_int_equal(4, set->count);

    ly_set_free(set, NULL);

    /* hashes used even for leaf-lists */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll2[. = 'three']", &set));
    assert_int_equal(1, set->count);

    node = set->objs[0];
    assert_string_equal(node->schema->name, "ll2");
    assert_string_equal(lyd_get_value(node), "three");

    ly_set_free(set, NULL);

    /* not found using hashes */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[a='val_d']", &set));
    assert_int_equal(0, set->count);

    ly_set_free(set, NULL);

    /* white-spaces are also ok */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/ll[ \na = 'val_c' ]", &set));
    assert_int_equal(1, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_rpc(void **state)
{
    const char *data =
            "<r xmlns=\"urn:tests:a\">\n"
            "    <l>val</l>\n"
            "</r>";
    struct ly_in *in;
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_REPLY_YANG, &tree, NULL));
    ly_in_free(in, 0);
    assert_non_null(tree);

    /* name collision input/output, hashes are not used */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:r/l", &set));
    assert_int_equal(1, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_toplevel(void **state)
{
    const char *schema_b =
            "module b {\n"
            "    namespace urn:tests:b;\n"
            "    prefix b;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    list l2 {\n"
            "        key \"a\";\n"
            "        leaf a {\n"
            "            type uint16;\n"
            "        }\n"
            "        leaf b {\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n"
            "}";
    const char *data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>\n"
            "<foo xmlns=\"urn:tests:a\">foo value</foo>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>1</a>\n"
            "    <b>1</b>\n"
            "</l2>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>2</a>\n"
            "    <b>1</b>\n"
            "</l2>\n"
            "<l2 xmlns=\"urn:tests:b\">\n"
            "    <a>3</a>\n"
            "    <b>1</b>\n"
            "</l2>";
    struct lyd_node *tree;
    struct ly_set *set;

    UTEST_ADD_MODULE(schema_b, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* all top-level nodes from one module (default container as well) */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:*", &set));
    assert_int_equal(5, set->count);

    ly_set_free(set, NULL);

    /* all top-level nodes from all modules */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/*", &set));
    assert_int_equal(8, set->count);

    ly_set_free(set, NULL);

    /* all nodes from one module */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//a:*", &set));
    assert_int_equal(13, set->count);

    ly_set_free(set, NULL);

    /* all nodes from all modules */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//*", &set));
    assert_int_equal(22, set->count);

    ly_set_free(set, NULL);

    /* all nodes from all modules #2 */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//.", &set));
    assert_int_equal(22, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_atomize(void **state)
{
    struct ly_set *set;
    const struct lys_module *mod;

    mod = ly_ctx_get_module_latest(UTEST_LYCTX, "a");
    assert_non_null(mod);

    /* some random paths just making sure the API function works */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/a:*", 0, &set));
    assert_int_equal(7, set->count);
    ly_set_free(set, NULL);

    /* all nodes from all modules (including internal, which can change easily, so check just the test modules) */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "//.", 0, &set));
    assert_in_range(set->count, 17, UINT32_MAX);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/a:c/ll[a='val1']/ll[a='val2']/b", 0, &set));
    assert_int_equal(6, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/ietf-interfaces:interfaces/*", 0, &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/*", 0, &set));
    assert_int_equal(14, set->count);
    ly_set_free(set, NULL);

    /*
     * axes
     */

    /* ancestor */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "//ll[a and b]/a/ancestor::node()", 0, &set));
    assert_int_equal(6, set->count);
    ly_set_free(set, NULL);

    /* ancestor-or-self */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "//ll[a and b]/ancestor-or-self::ll", 0, &set));
    assert_int_equal(5, set->count);
    ly_set_free(set, NULL);

    /* attribute */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/l1/attribute::key", 0, &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* child */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/child::l1/child::a", 0, &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    /* descendant */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/descendant::c/descendant::b", 0, &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* descendant-or-self */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/a:*/descendant-or-self::c", 0, &set));
    assert_int_equal(8, set->count);
    ly_set_free(set, NULL);

    /* following */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/c/x/following::a", 0, &set));
    assert_int_equal(4, set->count);
    ly_set_free(set, NULL);

    /* following-sibling */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/c/x/following-sibling::ll", 0, &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* parent */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/child::a:*/c/parent::l1", 0, &set));
    assert_int_equal(8, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/child::a:c//..", 0, &set));
    assert_int_equal(11, set->count);
    ly_set_free(set, NULL);

    /* preceding */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/c/preceding::a", 0, &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    /* preceding-sibling */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/c/ll/preceding-sibling::node()", 0, &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* self */
    assert_int_equal(LY_SUCCESS, lys_find_xpath_atoms(UTEST_LYCTX, NULL, "/c/self::c/ll/ll/b/self::b", 0, &set));
    assert_int_equal(4, set->count);
    ly_set_free(set, NULL);
}

static void
test_canonize(void **state)
{
    const char *data =
            "<foo2 xmlns=\"urn:tests:a\">50</foo2>"
            "<foo3 xmlns=\"urn:tests:a\" xmlns:a=\"urn:tests:a\">a:id_b</foo3>"
            "<foo4 xmlns=\"urn:tests:a\">250.5</foo4>";
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* integer */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo2[.='050']", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* identityref */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo3[.='id_b']", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    /* decimal64 */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo4[.='0250.500']", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_derived_from(void **state)
{
    const char *data =
            "<foo3 xmlns=\"urn:tests:a\">id_c</foo3>";
    struct lyd_node *tree;
    struct ly_set *set;

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo3[derived-from(., 'a:id_b')]", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:foo3[derived-from(., 'a:id_a')]", &set));
    assert_int_equal(1, set->count);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_augment(void **state)
{
    const char *schema_b =
            "module b {\n"
            "    namespace urn:tests:b;\n"
            "    prefix b;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    import a {\n"
            "        prefix a;\n"
            "    }\n"
            "\n"
            "    augment /a:c {\n"
            "        leaf a {\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n"
            "}";
    const char *data =
            "<c xmlns=\"urn:tests:a\">\n"
            "    <x>value</x>\n"
            "    <ll>\n"
            "        <a>key</a>\n"
            "    </ll>\n"
            "    <a xmlns=\"urn:tests:b\">25</a>\n"
            "    <ll2>c1</ll2>\n"
            "</c>";
    struct lyd_node *tree;
    struct ly_set *set;

    UTEST_ADD_MODULE(schema_b, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* get all children ignoring their module */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/a:c/*", &set));
    assert_int_equal(4, set->count);

    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_variables(void **state)
{
    struct lyd_node *tree, *node;
    struct ly_set *set;
    const char *data;
    struct lyxp_var *vars = NULL;

#define LOCAL_SETUP(DATA, TREE) \
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, DATA, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &TREE)); \
    assert_non_null(TREE);

#define SET_NODE(NODE, SET, INDEX) \
    assert_non_null(SET); \
    assert_true(INDEX < SET->count); \
    NODE = SET->objs[INDEX];

#define LOCAL_TEARDOWN(SET, TREE, VARS) \
    ly_set_free(SET, NULL); \
    lyd_free_all(TREE); \
    lyxp_vars_free(VARS); \
    vars = NULL;

    /* Eval variable to number. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "    <c>c2</c>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "2"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a2");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Eval variable to string. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "\"mstr\""));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/foo[text() = $var]", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "mstr");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Eval variable to set of nodes. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "    <c>c2</c>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "c"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a2");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable in union expr. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "    <c>c2</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a3</a>\n"
            "    <b>b3</b>\n"
            "    <c>c3</c>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "c[../a = 'a3']"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[c[../a = 'a1'] | $var]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a1");
    SET_NODE(node, set, 1);
    assert_string_equal(lyd_get_value(node), "a3");
    assert_int_equal(set->count, 2);
    LOCAL_TEARDOWN(set, tree, vars);

    /* Predicate after variable. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "    <c>c2</c>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "c"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var[../a = 'a1']]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a1");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable in variable. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "$var2"));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var2", "\"mstr\""));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/foo[text() = $var]", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "mstr");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Compare two variables. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "\"str\""));
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var2", "\"str\""));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/foo[$var1 = $var2]", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "mstr");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Arithmetic operation with variable. */
    data =
            "<foo2 xmlns=\"urn:tests:a\">4</foo2>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "2"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/foo2[.= ($var1 * 2)]", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "4");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable as function parameter. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "./c"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[count($var) = 1]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a1");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable in path expr. */
    /* NOTE: The variable can only be at the beginning of the expression path. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "/l1"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var/a]", vars, &set));
    assert_int_equal(set->count, 2);
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable as function. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "position()"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var = 2]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a2");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Dynamic change of value. */
    data =
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a1</a>\n"
            "    <b>b1</b>\n"
            "    <c>c1</c>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "    <a>a2</a>\n"
            "    <b>b2</b>\n"
            "    <c>c2</c>\n"
            "</l1>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "1"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a1");
    ly_set_free(set, NULL);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "2"));
    assert_int_equal(LY_SUCCESS, lyd_find_xpath2(tree, "/l1[$var]/a", vars, &set));
    SET_NODE(node, set, 0);
    assert_string_equal(lyd_get_value(node), "a2");
    LOCAL_TEARDOWN(set, tree, vars);

    /* Variable not defined. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var1", "\"mstr\""));
    assert_int_equal(LY_ENOTFOUND, lyd_find_xpath2(tree, "/foo[text() = $var55]", vars, &set));
    CHECK_LOG_CTX("Variable \"var55\" not defined.", NULL, 0);
    LOCAL_TEARDOWN(set, tree, vars);

    /* Syntax error in value. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "\""));
    assert_int_equal(LY_EVALID, lyd_find_xpath2(tree, "/foo[$var]", vars, &set));
    CHECK_LOG_CTX("Unterminated string delimited with \" (\").", "/a:foo", 0);
    LOCAL_TEARDOWN(set, tree, vars);

    /* Prefix is not supported. */
    data =
            "<foo xmlns=\"urn:tests:a\">mstr</foo>";
    LOCAL_SETUP(data, tree);
    assert_int_equal(LY_SUCCESS, lyxp_vars_set(&vars, "var", "\""));
    assert_int_equal(LY_EVALID, lyd_find_xpath2(tree, "/foo[$pref:var]", vars, &set));
    CHECK_LOG_CTX("Variable with prefix is not supported.", NULL, 0);
    LOCAL_TEARDOWN(set, tree, vars);

#undef LOCAL_SETUP
#undef LOCAL_TEARDOWN
}

static void
test_axes(void **state)
{
    const char *data;
    struct lyd_node *tree;
    struct ly_set *set;

    data =
            "<l1 xmlns=\"urn:tests:a\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\">\n"
            "  <a>a1</a>\n"
            "  <b yang:operation=\"replace\">b1</b>\n"
            "  <c yang:operation=\"none\">c1</c>\n"
            "</l1>\n"
            "<l1 xmlns=\"urn:tests:a\">\n"
            "  <a>a2</a>\n"
            "  <b>b2</b>\n"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\">\n"
            "  <a yang:operation=\"none\" yang:key=\"[no-key='no-value']\">a3</a>\n"
            "  <b>b3</b>\n"
            "  <c yang:value=\"no-val\">c3</c>\n"
            "</l1>"
            "<c xmlns=\"urn:tests:a\">"
            "  <x>val</x>"
            "  <ll>"
            "    <a>key1</a>"
            "    <ll>"
            "      <a>key11</a>"
            "      <b>val11</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key12</a>"
            "      <b>val12</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key13</a>"
            "      <b>val13</b>"
            "    </ll>"
            "  </ll>"
            "  <ll>"
            "    <a>key2</a>"
            "    <ll>"
            "      <a>key21</a>"
            "      <b>val21</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key22</a>"
            "      <b>val22</b>"
            "    </ll>"
            "  </ll>"
            "</c>";
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    /* ancestor */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//ll[a and b]/a/ancestor::node()", &set));
    assert_int_equal(8, set->count);
    ly_set_free(set, NULL);

    /* ancestor-or-self */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//ll[a and b]/ancestor-or-self::ll", &set));
    assert_int_equal(7, set->count);
    ly_set_free(set, NULL);

    /* attribute */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/l1/@operation", &set));
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/l1/attribute::key", &set));
    assert_int_equal(0, set->count);
    ly_set_free(set, NULL);

    /* child */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/child::l1/child::a", &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* descendant */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/descendant::c/descendant::b", &set));
    assert_int_equal(5, set->count);
    ly_set_free(set, NULL);

    /* descendant-or-self */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "//c", &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/descendant-or-self::node()/c", &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* following */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/x/following::a", &set));
    assert_int_equal(7, set->count);
    ly_set_free(set, NULL);

    /* following-sibling */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/x/following-sibling::ll", &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    /* parent */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/child::*/c/parent::l1", &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/child::c//..", &set));
    assert_int_equal(8, set->count);
    ly_set_free(set, NULL);

    /* preceding */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/preceding::a", &set));
    assert_int_equal(3, set->count);
    ly_set_free(set, NULL);

    /* preceding-sibling */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/ll/preceding-sibling::node()", &set));
    assert_int_equal(2, set->count);
    ly_set_free(set, NULL);

    /* self */
    assert_int_equal(LY_SUCCESS, lyd_find_xpath(tree, "/c/self::c/ll/ll/b/self::b", &set));
    assert_int_equal(5, set->count);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

static void
test_trim(void **state)
{
    const char *data;
    char *str1;
    struct lyd_node *tree;

    data =
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a1</a>"
            "  <b>b1</b>"
            "  <c>c1</c>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a2</a>"
            "  <b>b2</b>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a3</a>"
            "  <b>b3</b>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a4</a>"
            "  <b>b4</b>"
            "  <c>c4</c>"
            "</l1>"
            "<l1 xmlns=\"urn:tests:a\">"
            "  <a>a5</a>"
            "  <b>b5</b>"
            "  <c>c5</c>"
            "</l1>"
            "<foo2 xmlns=\"urn:tests:a\">50</foo2>"
            "<c xmlns=\"urn:tests:a\">"
            "  <x>key2</x>"
            "  <ll>"
            "    <a>key1</a>"
            "    <ll>"
            "      <a>key11</a>"
            "      <b>val11</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key12</a>"
            "      <b>val12</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key13</a>"
            "      <b>val13</b>"
            "    </ll>"
            "  </ll>"
            "  <ll>"
            "    <a>key2</a>"
            "    <ll>"
            "      <a>key21</a>"
            "      <b>val21</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key22</a>"
            "      <b>val22</b>"
            "    </ll>"
            "  </ll>"
            "  <ll>"
            "    <a>key3</a>"
            "    <ll>"
            "      <a>key31</a>"
            "      <b>val31</b>"
            "    </ll>"
            "    <ll>"
            "      <a>key32</a>"
            "      <b>val32</b>"
            "    </ll>"
            "  </ll>"
            "</c>";

    /* trim #1 */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_SUCCESS, lyd_trim_xpath(&tree, "/a:c/ll/ll[a='key11']", NULL));
    lyd_print_mem(&str1, tree, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(str1,
            "<c xmlns=\"urn:tests:a\">\n"
            "  <ll>\n"
            "    <a>key1</a>\n"
            "    <ll>\n"
            "      <a>key11</a>\n"
            "      <b>val11</b>\n"
            "    </ll>\n"
            "  </ll>\n"
            "</c>\n");

    free(str1);
    lyd_free_all(tree);

    /* trim #2 */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_SUCCESS, lyd_trim_xpath(&tree, "/a:c/ll/ll[contains(.,'2')]", NULL));
    lyd_print_mem(&str1, tree, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(str1,
            "<c xmlns=\"urn:tests:a\">\n"
            "  <ll>\n"
            "    <a>key1</a>\n"
            "    <ll>\n"
            "      <a>key12</a>\n"
            "      <b>val12</b>\n"
            "    </ll>\n"
            "  </ll>\n"
            "  <ll>\n"
            "    <a>key2</a>\n"
            "    <ll>\n"
            "      <a>key21</a>\n"
            "      <b>val21</b>\n"
            "    </ll>\n"
            "    <ll>\n"
            "      <a>key22</a>\n"
            "      <b>val22</b>\n"
            "    </ll>\n"
            "  </ll>\n"
            "  <ll>\n"
            "    <a>key3</a>\n"
            "    <ll>\n"
            "      <a>key32</a>\n"
            "      <b>val32</b>\n"
            "    </ll>\n"
            "  </ll>\n"
            "</c>\n");

    free(str1);
    lyd_free_all(tree);

    /* trim #3 */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    assert_int_equal(LY_SUCCESS, lyd_trim_xpath(&tree, "/l1[4]//.", NULL));
    lyd_print_mem(&str1, tree, LYD_XML, LYD_PRINT_WITHSIBLINGS);
    assert_string_equal(str1,
            "<l1 xmlns=\"urn:tests:a\">\n"
            "  <a>a4</a>\n"
            "  <b>b4</b>\n"
            "  <c>c4</c>\n"
            "</l1>\n");

    free(str1);
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_predicate, setup),
        UTEST(test_union, setup),
        UTEST(test_invalid, setup),
        UTEST(test_hash, setup),
        UTEST(test_rpc, setup),
        UTEST(test_toplevel, setup),
        UTEST(test_atomize, setup),
        UTEST(test_canonize, setup),
        UTEST(test_derived_from, setup),
        UTEST(test_augment, setup),
        UTEST(test_variables, setup),
        UTEST(test_axes, setup),
        UTEST(test_trim, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
