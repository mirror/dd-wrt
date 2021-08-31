/**
 * @file test_validation.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from validation.c
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

#include <stdio.h>
#include <string.h>

#include "context.h"
#include "in.h"
#include "out.h"
#include "parser_data.h"
#include "printer_data.h"
#include "tests_config.h"
#include "tree_data_internal.h"
#include "tree_schema.h"

#define LYD_TREE_CREATE(INPUT, MODEL) \
                CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, MODEL)

static void
test_when(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module a {\n"
            "    namespace urn:tests:a;\n"
            "    prefix a;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        leaf a {\n"
            "            when \"../../c = 'val_c'\";\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf c {\n"
            "        when \"/cont/b = 'val_b'\";\n"
            "        type string;\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<c xmlns=\"urn:tests:a\">hey</c>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("When condition \"/cont/b = 'val_b'\" not satisfied.", "Schema location /a:c, data location /a:c.");

    LYD_TREE_CREATE("<cont xmlns=\"urn:tests:a\"><b>val_b</b></cont><c xmlns=\"urn:tests:a\">hey</c>", tree);
    CHECK_LYSC_NODE(tree->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 0, LYS_LEAF, 0, 0, NULL, 1);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);

    LYD_TREE_CREATE("<cont xmlns=\"urn:tests:a\"><a>val</a><b>val_b</b></cont><c xmlns=\"urn:tests:a\">val_c</c>", tree);
    CHECK_LYSC_NODE(lyd_child(tree)->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "a", 1, LYS_LEAF, 1, 0, NULL, 1);
    assert_int_equal(LYD_WHEN_TRUE, lyd_child(tree)->flags);
    CHECK_LYSC_NODE(tree->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "c", 0, LYS_LEAF, 0, 0, NULL, 1);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->flags);
    lyd_free_all(tree);
}

static void
test_mandatory_when(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module a {\n"
            "    namespace urn:tests:a;\n"
            "    prefix a;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            when \"../a = 'val_a'\";\n"
            "            mandatory true;\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf c {\n"
            "        type string;\n"
            "    }\n"
            "    leaf d {\n"
            "        when \"../c = 'val_c'\";\n"
            "        mandatory true;\n"
            "        type string;\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<d xmlns=\"urn:tests:a\">hey</d>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("When condition \"../c = 'val_c'\" not satisfied.", "Schema location /a:d, data location /a:d.");

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:a\"><b>hey</b></cont>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("When condition \"../a = 'val_a'\" not satisfied.", "Schema location /a:cont/b, data location /a:cont/b.");

    LYD_TREE_CREATE("<c xmlns=\"urn:tests:a\">val_c</c><d xmlns=\"urn:tests:a\">hey</d>", tree);
    CHECK_LYSC_NODE(tree->next->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE, 1, "d", 0, LYS_LEAF, 0, 0, NULL, 1);
    assert_int_equal(LYD_WHEN_TRUE, tree->next->next->flags);
    lyd_free_all(tree);

    LYD_TREE_CREATE("<cont xmlns=\"urn:tests:a\"><a>val_a</a><b>hey</b></cont>", tree);
    CHECK_LYSC_NODE(lyd_child(tree)->next->schema, NULL, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE, 1, "b", 0, LYS_LEAF, tree->schema, 0, NULL, 1);
    assert_int_equal(LYD_WHEN_TRUE, lyd_child(tree)->next->flags);
    lyd_free_all(tree);
}

static void
test_mandatory(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module b {\n"
            "    namespace urn:tests:b;\n"
            "    prefix b;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        mandatory true;\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    leaf c {\n"
            "        mandatory true;\n"
            "        type string;\n"
            "    }\n"
            "    leaf d {\n"
            "        type empty;\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<d xmlns=\"urn:tests:b\"/>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Mandatory choice \"choic\" data do not exist.", "Schema location /b:choic.");

    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:b\">string</l><d xmlns=\"urn:tests:b\"/>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Mandatory node \"c\" instance does not exist.", "Schema location /b:c.");

    CHECK_PARSE_LYD_PARAM("<a xmlns=\"urn:tests:b\">string</a>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Mandatory node \"c\" instance does not exist.", "Schema location /b:c.");

    LYD_TREE_CREATE("<a xmlns=\"urn:tests:b\">string</a><c xmlns=\"urn:tests:b\">string2</c>", tree);
    lyd_free_siblings(tree);
}

static void
test_minmax(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module c {\n"
            "    namespace urn:tests:c;\n"
            "    prefix c;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf-list l {\n"
            "                min-elements 3;\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    list lt {\n"
            "        max-elements 4;\n"
            "        key \"k\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type empty;\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:c\">mate</l>"
            "<d xmlns=\"urn:tests:c\"/>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Too few \"l\" instances.", "Schema location /c:choic/b/l.");

    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Too few \"l\" instances.", "Schema location /c:choic/b/l.");

    LYD_TREE_CREATE("<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>"
            "<l xmlns=\"urn:tests:c\">val3</l>", tree);
    lyd_free_all(tree);

    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:c\">val1</l>"
            "<l xmlns=\"urn:tests:c\">val2</l>"
            "<l xmlns=\"urn:tests:c\">val3</l>"
            "<lt xmlns=\"urn:tests:c\"><k>val1</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val2</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val3</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val4</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val5</k></lt>"
            "<lt xmlns=\"urn:tests:c\"><k>val6</k></lt>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Too many \"lt\" instances.", "Schema location /c:lt, data location /c:lt[k='val5'].");
}

const char *schema_d =
        "module d {\n"
        "    namespace urn:tests:d;\n"
        "    prefix d;\n"
        "    yang-version 1.1;\n"
        "\n"
        "    list lt {\n"
        "        key \"k\";\n"
        "        unique \"l1\";\n"
        "        leaf k {\n"
        "            type string;\n"
        "        }\n"
        "        leaf l1 {\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        "    list lt2 {\n"
        "        key \"k\";\n"
        "        unique \"cont/l2 l4\";\n"
        "        unique \"l5 l6\";\n"
        "        leaf k {\n"
        "            type string;\n"
        "        }\n"
        "        container cont {\n"
        "            leaf l2 {\n"
        "                type string;\n"
        "            }\n"
        "        }\n"
        "        leaf l4 {\n"
        "            type string;\n"
        "        }\n"
        "        leaf l5 {\n"
        "            type string;\n"
        "        }\n"
        "        leaf l6 {\n"
        "            type string;\n"
        "        }\n"
        "        list lt3 {\n"
        "            key \"kk\";\n"
        "            unique \"l3\";\n"
        "            leaf kk {\n"
        "                type string;\n"
        "            }\n"
        "            leaf l3 {\n"
        "                type string;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}";

static void
test_unique(void **state)
{
    struct lyd_node *tree;

    UTEST_ADD_MODULE(schema_d, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "</lt>", tree);
    lyd_free_all(tree);

    LYD_TREE_CREATE("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>not-same</l1>\n"
            "</lt>", tree);
    lyd_free_all(tree);

    CHECK_PARSE_LYD_PARAM("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>same</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>same</l1>\n"
            "</lt>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val1']\" and \"/d:lt[k='val2']\".",
            "Schema location /d:lt, data location /d:lt[k='val2'].");

    /* now try with more instances */
    LYD_TREE_CREATE("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <l1>3</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <l1>4</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <l1>5</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "    <l1>6</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "    <l1>7</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "    <l1>8</l1>\n"
            "</lt>", tree);
    lyd_free_all(tree);

    LYD_TREE_CREATE("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <l1>3</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <l1>5</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "    <l1>6</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "</lt>", tree);
    lyd_free_all(tree);

    CHECK_PARSE_LYD_PARAM("<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <l1>1</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <l1>4</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val6</k>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val7</k>\n"
            "    <l1>2</l1>\n"
            "</lt>\n"
            "<lt xmlns=\"urn:tests:d\">\n"
            "    <k>val8</k>\n"
            "    <l1>8</l1>\n"
            "</lt>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"l1\" not satisfied in \"/d:lt[k='val7']\" and \"/d:lt[k='val2']\".",
            "Schema location /d:lt, data location /d:lt[k='val2'].");
}

static void
test_unique_nested(void **state)
{
    struct lyd_node *tree;

    UTEST_ADD_MODULE(schema_d, LYS_IN_YANG, NULL, NULL);

    /* nested list uniquest are compared only with instances in the same parent list instance */
    LYD_TREE_CREATE("<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val2</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>3</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <l4>4</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>5</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>", tree);
    lyd_free_all(tree);

    CHECK_PARSE_LYD_PARAM("<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val2</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "    <lt3>\n"
            "        <kk>val3</kk>\n"
            "        <l3>1</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>2</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <lt3>\n"
            "        <kk>val1</kk>\n"
            "        <l3>3</l3>\n"
            "    </lt3>\n"
            "</lt2>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"l3\" not satisfied in \"/d:lt2[k='val2']/lt3[kk='val3']\" and \"/d:lt2[k='val2']/lt3[kk='val1']\".",
            "Schema location /d:lt2/lt3, data location /d:lt2[k='val2']/lt3[kk='val1'].");

    CHECK_PARSE_LYD_PARAM("<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>3</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>2</l4>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>5</l4>\n"
            "</lt2>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"cont/l2 l4\" not satisfied in \"/d:lt2[k='val4']\" and \"/d:lt2[k='val2']\".",
            "Schema location /d:lt2, data location /d:lt2[k='val2'].");

    CHECK_PARSE_LYD_PARAM("<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val1</k>\n"
            "    <cont>\n"
            "        <l2>1</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>1</l5>\n"
            "    <l6>1</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val2</k>\n"
            "    <cont>\n"
            "        <l2>2</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>1</l5>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val3</k>\n"
            "    <cont>\n"
            "        <l2>3</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>3</l5>\n"
            "    <l6>3</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val4</k>\n"
            "    <cont>\n"
            "        <l2>4</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l6>1</l6>\n"
            "</lt2>\n"
            "<lt2 xmlns=\"urn:tests:d\">\n"
            "    <k>val5</k>\n"
            "    <cont>\n"
            "        <l2>5</l2>\n"
            "    </cont>\n"
            "    <l4>1</l4>\n"
            "    <l5>3</l5>\n"
            "    <l6>3</l6>\n"
            "</lt2>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"l5 l6\" not satisfied in \"/d:lt2[k='val5']\" and \"/d:lt2[k='val3']\".",
            "Schema location /d:lt2, data location /d:lt2[k='val3'].");
}

static void
test_dup(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module e {\n"
            "    namespace urn:tests:e;\n"
            "    prefix e;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf-list l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    list lt {\n"
            "        key \"k\";\n"
            "        leaf k {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type uint32;\n"
            "    }\n"
            "    leaf-list ll {\n"
            "        type string;\n"
            "    }\n"
            "    container cont {\n"
            "        list lt {\n"
            "            key \"k\";\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "        leaf d {\n"
            "            type uint32;\n"
            "        }\n"
            "        leaf-list ll {\n"
            "            type string;\n"
            "        }\n"
            "        leaf-list ll2 {\n"
            "            type enumeration {\n"
            "                enum one;\n"
            "                enum two;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<d xmlns=\"urn:tests:e\">25</d><d xmlns=\"urn:tests:e\">50</d>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"d\".", "Schema location /e:d, data location /e:d.");

    CHECK_PARSE_LYD_PARAM("<lt xmlns=\"urn:tests:e\"><k>A</k></lt>"
            "<lt xmlns=\"urn:tests:e\"><k>B</k></lt>"
            "<lt xmlns=\"urn:tests:e\"><k>A</k></lt>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"lt\".", "Schema location /e:lt, data location /e:lt[k='A'].");

    CHECK_PARSE_LYD_PARAM("<ll xmlns=\"urn:tests:e\">A</ll>"
            "<ll xmlns=\"urn:tests:e\">B</ll>"
            "<ll xmlns=\"urn:tests:e\">B</ll>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"ll\".", "Schema location /e:ll, data location /e:ll[.='B'].");

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:e\"></cont><cont xmlns=\"urn:tests:e\"/>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"cont\".", "Schema location /e:cont, data location /e:cont.");

    /* same tests again but using hashes */
    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:e\"><d>25</d><d>50</d><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll></cont>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"d\".", "Schema location /e:cont/d, data location /e:cont/d, line number 1.");

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
            "<lt><k>a</k></lt>"
            "<lt><k>b</k></lt>"
            "<lt><k>c</k></lt>"
            "<lt><k>d</k></lt>"
            "<lt><k>c</k></lt></cont>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"lt\".", "Schema location /e:cont/lt, data location /e:cont/lt[k='c'], line number 1.");

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:e\"><ll>1</ll><ll>2</ll><ll>3</ll><ll>4</ll>"
            "<ll>a</ll><ll>b</ll><ll>c</ll><ll>d</ll><ll>d</ll></cont>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"ll\".", "Schema location /e:cont/ll, data location /e:cont/ll[.='d'], line number 1.");

    /* cases */
    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:e\">a</l>"
            "<l xmlns=\"urn:tests:e\">b</l>"
            "<l xmlns=\"urn:tests:e\">c</l>"
            "<l xmlns=\"urn:tests:e\">b</l>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Duplicate instance of \"l\".", "Schema location /e:choic/b/l, data location /e:l[.='b'].");

    CHECK_PARSE_LYD_PARAM("<l xmlns=\"urn:tests:e\">a</l><l xmlns=\"urn:tests:e\">b</l>"
            "<l xmlns=\"urn:tests:e\">c</l>"
            "<a xmlns=\"urn:tests:e\">aa</a>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Data for both cases \"a\" and \"b\" exist.", "Schema location /e:choic.");
}

static void
test_defaults(void **state)
{
    struct lyd_node *tree, *node, *diff;
    const struct lys_module *mod;
    const char *schema =
            "module f {\n"
            "    namespace urn:tests:f;\n"
            "    prefix f;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    choice choic {\n"
            "        default \"c\";\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        case b {\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "        case c {\n"
            "            leaf-list ll1 {\n"
            "                type string;\n"
            "                default \"def1\";\n"
            "                default \"def2\";\n"
            "                default \"def3\";\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    leaf d {\n"
            "        type uint32;\n"
            "        default 15;\n"
            "    }\n"
            "    leaf dd {\n"
            "        type uint32;\n"
            "        when '../d = 666';\n"
            "        default 15;\n"
            "    }\n"
            "    leaf-list ll2 {\n"
            "        type string;\n"
            "        default \"dflt1\";\n"
            "        default \"dflt2\";\n"
            "    }\n"
            "    container cont {\n"
            "        choice choic {\n"
            "            default \"c\";\n"
            "            leaf a {\n"
            "                type string;\n"
            "            }\n"
            "            case b {\n"
            "                leaf l {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "            case c {\n"
            "                leaf-list ll1 {\n"
            "                    type string;\n"
            "                    default \"def1\";\n"
            "                    default \"def2\";\n"
            "                    default \"def3\";\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        leaf d {\n"
            "            type uint32;\n"
            "            default 15;\n"
            "        }\n"
            "        leaf dd {\n"
            "            type uint32;\n"
            "            when '../d = 666';\n"
            "            default 15;\n"
            "        }\n"
            "        leaf-list ll2 {\n"
            "            type string;\n"
            "            default \"dflt1\";\n"
            "            default \"dflt2\";\n"
            "        }\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-with-defaults", "2011-06-01", NULL));\

    /* get defaults */
    tree = NULL;
    assert_int_equal(lyd_validate_module(&tree, mod, 0, &diff), LY_SUCCESS);
    assert_non_null(tree);
    assert_non_null(diff);

    /* check all defaults exist */
    CHECK_LYD_STRING_PARAM(tree,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_WITHSIBLINGS);

    /* check diff */
    CHECK_LYD_STRING_PARAM(diff,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">def3</ll1>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <ll1 yang:operation=\"create\">def1</ll1>\n"
            "  <ll1 yang:operation=\"create\">def2</ll1>\n"
            "  <ll1 yang:operation=\"create\">def3</ll1>\n"
            "  <d yang:operation=\"create\">15</d>\n"
            "  <ll2 yang:operation=\"create\">dflt1</ll2>\n"
            "  <ll2 yang:operation=\"create\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(diff);

    /* create another explicit case and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "l", "value", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, UTEST_LYCTX, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    CHECK_LYD_STRING_PARAM(tree,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_WITHSIBLINGS);

    /* check diff */
    CHECK_LYD_STRING_PARAM(diff,
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">def3</ll1>\n",
            LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(diff);

    /* create explicit leaf-list and leaf and validate */
    assert_int_equal(lyd_new_term(NULL, mod, "d", "15", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll2", "dflt2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, UTEST_LYCTX, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    CHECK_LYD_STRING_PARAM(tree,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_WITHSIBLINGS);

    /* check diff */
    CHECK_LYD_STRING_PARAM(diff,
            "<d xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">dflt2</ll2>\n",
            LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(diff);

    /* create first explicit container, which should become implicit */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, UTEST_LYCTX, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    CHECK_LYD_STRING_PARAM(tree,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_WITHSIBLINGS);
    /* check diff */
    assert_null(diff);

    /* create second explicit container, which should become implicit, so the first tree node should be removed */
    assert_int_equal(lyd_new_inner(NULL, mod, "cont", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(tree, node, &tree), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, UTEST_LYCTX, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    CHECK_LYD_STRING_PARAM(tree,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def1</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def2</ll1>\n"
            "  <ll1 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">def3</ll1>\n"
            "  <d xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">15</d>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt1</ll2>\n"
            "  <ll2 xmlns:ncwd=\"urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults\" ncwd:default=\"true\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_IMPL_TAG | LYD_PRINT_WITHSIBLINGS);
    /* check diff */
    assert_null(diff);

    /* similar changes for nested defaults */
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll1", "def3", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "d", "5", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(tree->prev, NULL, "ll2", "non-dflt", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&tree, UTEST_LYCTX, LYD_VALIDATE_PRESENT, &diff), LY_SUCCESS);

    /* check data tree */
    CHECK_LYD_STRING_PARAM(tree,
            "<l xmlns=\"urn:tests:f\">value</l>\n"
            "<d xmlns=\"urn:tests:f\">15</d>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1>def3</ll1>\n"
            "  <d>5</d>\n"
            "  <ll2>non-dflt</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WITHSIBLINGS);

    /* check diff */
    CHECK_LYD_STRING_PARAM(diff,
            "<cont xmlns=\"urn:tests:f\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ll1 yang:operation=\"delete\">def1</ll1>\n"
            "  <ll1 yang:operation=\"delete\">def2</ll1>\n"
            "  <ll1 yang:operation=\"delete\">def3</ll1>\n"
            "  <d yang:operation=\"delete\">15</d>\n"
            "  <ll2 yang:operation=\"delete\">dflt1</ll2>\n"
            "  <ll2 yang:operation=\"delete\">dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(diff);
    lyd_free_all(tree);

    /* check data tree - when enabled node */
    CHECK_PARSE_LYD_PARAM("<d xmlns=\"urn:tests:f\">666</d><cont xmlns=\"urn:tests:f\"><d>666</d></cont>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    CHECK_LYD_STRING_PARAM(tree,
            "<ll1 xmlns=\"urn:tests:f\">def1</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\">def2</ll1>\n"
            "<ll1 xmlns=\"urn:tests:f\">def3</ll1>\n"
            "<d xmlns=\"urn:tests:f\">666</d>\n"
            "<dd xmlns=\"urn:tests:f\">15</dd>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt1</ll2>\n"
            "<ll2 xmlns=\"urn:tests:f\">dflt2</ll2>\n"
            "<cont xmlns=\"urn:tests:f\">\n"
            "  <ll1>def1</ll1>\n"
            "  <ll1>def2</ll1>\n"
            "  <ll1>def3</ll1>\n"
            "  <d>666</d>\n"
            "  <dd>15</dd>\n"
            "  <ll2>dflt1</ll2>\n"
            "  <ll2>dflt2</ll2>\n"
            "</cont>\n",
            LYD_XML, LYD_PRINT_WD_ALL | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(tree);
}

static void
test_state(void **state)
{
    const char *data;
    struct lyd_node *tree;
    const char *schema =
            "module h {\n"
            "    namespace urn:tests:h;\n"
            "    prefix h;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        container cont2 {\n"
            "            config false;\n"
            "            leaf l {\n"
            "                type string;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    data = "<cont xmlns=\"urn:tests:h\">\n"
            "  <cont2>\n"
            "    <l>val</l>\n"
            "  </cont2>\n"
            "</cont>\n";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_NO_STATE, 0, LY_EVALID, tree);
    CHECK_LOG_CTX("Unexpected data state node \"cont2\" found.",
            "Schema location /h:cont/cont2, data location /h:cont, line number 3.");

    CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);
    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT | LYD_VALIDATE_NO_STATE, NULL));
    CHECK_LOG_CTX("Unexpected data state node \"cont2\" found.",
            "Schema location /h:cont/cont2, data location /h:cont/cont2.");
    lyd_free_all(tree);
}

static void
test_must(void **state)
{
    struct lyd_node *tree;
    const char *schema =
            "module i {\n"
            "    namespace urn:tests:i;\n"
            "    prefix i;\n"
            "    yang-version 1.1;\n"
            "\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            type string;\n"
            "        }\n"
            "        leaf l2 {\n"
            "            must \"../l = 'right'\";\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:i\">\n"
            "  <l>wrong</l>\n"
            "  <l2>val</l2>\n"
            "</cont>\n", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Must condition \"../l = 'right'\" not satisfied.",
            "Schema location /i:cont/l2, data location /i:cont/l2.");

    LYD_TREE_CREATE("<cont xmlns=\"urn:tests:i\">\n"
            "  <l>right</l>\n"
            "  <l2>val</l2>\n"
            "</cont>\n", tree);
    lyd_free_all(tree);
}

const char *schema_j =
        "module j {\n"
        "    namespace urn:tests:j;\n"
        "    prefix j;\n"
        "    yang-version 1.1;\n"
        "\n"
        "    feature feat1;\n"
        "\n"
        "    container cont {\n"
        "        must \"false()\";\n"
        "        list l1 {\n"
        "            key \"k\";\n"
        "            leaf k {\n"
        "                type string;\n"
        "            }\n"
        "            action act {\n"
        "                if-feature feat1;\n"
        "                input {\n"
        "                    must \"../../lf1 = 'true'\";\n"
        "                    leaf lf2 {\n"
        "                        type leafref {\n"
        "                            path /lf3;\n"
        "                        }\n"
        "                    }\n"
        "                }\n"
        "                output {\n"
        "                    must \"../../lf1 = 'true2'\";\n"
        "                    leaf lf2 {\n"
        "                        type leafref {\n"
        "                            path /lf4;\n"
        "                        }\n"
        "                    }\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "\n"
        "        leaf lf1 {\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    leaf lf3 {\n"
        "        type string;\n"
        "    }\n"
        "\n"
        "    leaf lf4 {\n"
        "        type string;\n"
        "    }\n"
        "}";
const char *feats_j[] = {"feat1", NULL};

static void
test_action(void **state)
{
    struct ly_in *in;
    struct lyd_node *tree, *op_tree;

    UTEST_ADD_MODULE(schema_j, LYS_IN_YANG, feats_j, NULL);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <l1>\n"
            "    <k>val1</k>\n"
            "    <act>\n"
            "      <lf2>target</lf2>\n"
            "    </act>\n"
            "  </l1>\n"
            "</cont>\n", &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_RPC_YANG, &op_tree, NULL));
    assert_non_null(op_tree);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_TYPE_RPC_YANG, NULL));
    CHECK_LOG_CTX("Invalid leafref value \"target\" - no existing target instance \"/lf3\".",
            "Schema location /j:cont/l1/act/input/lf2, data location /j:cont/l1[k='val1']/act/lf2.");
    ly_in_free(in, 0);

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>not true</lf1>\n"
            "</cont>\n"
            "<lf3 xmlns=\"urn:tests:j\">target</lf3>\n",
            LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_TYPE_RPC_YANG, NULL));
    CHECK_LOG_CTX("Must condition \"../../lf1 = 'true'\" not satisfied.",
            "Data location /j:cont/l1[k='val1']/act.");

    lyd_free_all(tree);
    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>true</lf1>\n"
            "</cont>\n"
            "<lf3 xmlns=\"urn:tests:j\">target</lf3>\n",
            LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_TYPE_RPC_YANG, NULL));

    lyd_free_tree(op_tree);
    lyd_free_siblings(tree);
}

static void
test_reply(void **state)
{
    struct ly_in *in;
    struct lyd_node *tree, *op_tree;

    UTEST_ADD_MODULE(schema_j, LYS_IN_YANG, feats_j, NULL);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(
            "<cont xmlns=\"urn:tests:j\">\n"
            "  <l1>\n"
            "    <k>val1</k>\n"
            "    <act>\n"
            "      <lf2>target</lf2>\n"
            "    </act>\n"
            "  </l1>\n"
            "</cont>\n", &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_REPLY_YANG, &op_tree, NULL));
    assert_non_null(op_tree);
    ly_in_free(in, 0);

    /* missing leafref */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, NULL, LYD_TYPE_REPLY_YANG, NULL));
    CHECK_LOG_CTX("Invalid leafref value \"target\" - no existing target instance \"/lf4\".",
            "Schema location /j:cont/l1/act/output/lf2, data location /j:cont/l1[k='val1']/act/lf2.");

    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>not true</lf1>\n"
            "</cont>\n"
            "<lf4 xmlns=\"urn:tests:j\">target</lf4>\n",
            LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);

    /* input must false */
    assert_int_equal(LY_EVALID, lyd_validate_op(op_tree, tree, LYD_TYPE_REPLY_YANG, NULL));
    CHECK_LOG_CTX("Must condition \"../../lf1 = 'true2'\" not satisfied.", "Data location /j:cont/l1[k='val1']/act.");

    lyd_free_all(tree);
    CHECK_PARSE_LYD_PARAM("<cont xmlns=\"urn:tests:j\">\n"
            "  <lf1>true2</lf1>\n"
            "</cont>\n"
            "<lf4 xmlns=\"urn:tests:j\">target</lf4>\n",
            LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, tree);

    /* success */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op_tree, tree, LYD_TYPE_REPLY_YANG, NULL));

    lyd_free_tree(op_tree);
    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_when),
        UTEST(test_mandatory),
        UTEST(test_mandatory_when),
        UTEST(test_minmax),
        UTEST(test_unique),
        UTEST(test_unique_nested),
        UTEST(test_dup),
        UTEST(test_defaults),
        UTEST(test_state),
        UTEST(test_must),
        UTEST(test_action),
        UTEST(test_reply),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
