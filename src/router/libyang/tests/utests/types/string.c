/**
 * @file string.c
 * @author Radek Iša <isa@cesnet.cz>
 * @brief test for string values
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define  _UTEST_MAIN_
#include "../utests.h"

/* GLOBAL INCLUDE HEADERS */
#include <ctype.h>

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "path.h"
#include "plugins_internal.h"

#define MODULE_CREATE_YIN(MOD_NAME, NODES) \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
    "<module name=\"" MOD_NAME "\"\n" \
    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n" \
    "        xmlns:pref=\"urn:tests:" MOD_NAME "\">\n" \
    "  <yang-version value=\"1.1\"/>\n" \
    "  <namespace uri=\"urn:tests:" MOD_NAME "\"/>\n" \
    "  <prefix value=\"pref\"/>\n" \
    NODES \
    "</module>\n"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML(MOD_NAME, DATA, TYPE, ...)\
    {\
        struct lyd_node *tree;\
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>";\
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);\
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0);\
        CHECK_LYD_NODE_TERM((struct lyd_node_term *) tree, 0, 0, 0, 0, 1, TYPE,  __VA_ARGS__);\
        lyd_free_all(tree);\
    }

#define TEST_SUCCESS_JSON(MOD_NAME, DATA, TYPE, ...)\
    {\
        struct lyd_node *tree;\
        const char *data = "{\"" MOD_NAME ":port\":\"" DATA "\"}";\
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);\
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0);\
        CHECK_LYD_NODE_TERM((struct lyd_node_term *) tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__);\
        lyd_free_all(tree);\
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree_1; \
        struct lyd_node *tree_2; \
        char *xml_out, *data; \
        data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

#define TEST_ERROR_XML(MOD_NAME, DATA)\
    {\
        struct lyd_node *tree;\
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>";\
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);\
        assert_null(tree);\
    }

#define TEST_ERROR_JSON(MOD_NAME, DATA)\
    {\
        struct lyd_node *tree;\
        const char *data = "{\"" MOD_NAME ":port\":\"" DATA "\"}";\
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);\
        assert_null(tree);\
    }

static void
test_schema_yang(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_pattern *pattern;
    struct lysc_range *range;

    /* TEST BASE STRING */
    schema = MODULE_CREATE_YANG("base", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_STRING, 0, 0);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "string", 0, 0, 1, 0, 0, 0);

    /* TEST MODULE T0 */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type string"
            "{length \"10 .. max\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_true(range->parts[0].max_u64 == 18446744073709551615ull);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "10 .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T1 */
    schema = MODULE_CREATE_YANG("T1", "leaf port {type string"
            "{length \"min .. 20 | 50\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 0);
    assert_int_equal(range->parts[0].max_u64, 20);
    assert_int_equal(range->parts[1].min_u64, 50);
    assert_int_equal(range->parts[1].max_u64, 50);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "min .. 20 | 50", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T2 */
    schema = MODULE_CREATE_YANG("T2", "leaf port {type string"
            "{length \"10 .. 20 | 50 .. 100 | 255\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_int_equal(range->parts[0].max_u64, 20);
    assert_int_equal(range->parts[1].min_u64, 50);
    assert_int_equal(range->parts[1].max_u64, 100);
    assert_int_equal(range->parts[2].min_u64, 255);
    assert_int_equal(range->parts[2].max_u64, 255);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "10 .. 20 | 50 .. 100 | 255", NULL, NULL, NULL, 0, NULL);

    /* SUBTYPE MODULE T2 */
    schema = MODULE_CREATE_YANG("TS0",
            "typedef my_type {"
            "    type string {length \"10 .. 20 | 50 .. 100 | 255\";}"
            "}"
            "leaf port {type my_type {length \"min .. 15 | max\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_int_equal(range->parts[0].max_u64, 15);
    assert_int_equal(range->parts[1].min_u64, 255);
    assert_int_equal(range->parts[1].max_u64, 255);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "my_type", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "min .. 15 | max", NULL, NULL, NULL, 0, NULL);

    /* ERROR TESTS NEGATIVE VALUE */
    schema = MODULE_CREATE_YANG("ERR0", "leaf port {type string {length \"-1 .. 20\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid length restriction - value \"-1\" does not fit the type limitations.", "/ERR0:port", 0);

    schema = MODULE_CREATE_YANG("ERR1", "leaf port {type string {length \"100 .. 18446744073709551616\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid length restriction - invalid value \"18446744073709551616\".", "/ERR1:port", 0);

    schema = MODULE_CREATE_YANG("ERR2", "leaf port {type string {length \"10 .. 20 | 20 .. 30\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EEXIST);
    CHECK_LOG_CTX("Invalid length restriction - values are not in ascending order (20).", "/ERR2:port", 0);

    schema = MODULE_CREATE_YANG("ERR3",
            "typedef my_type {"
            "    type string;"
            "}"
            "leaf port {type my_type {length \"-1 .. 15\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid length restriction - value \"-1\" does not fit the type limitations.", "/ERR3:port", 0);

    /*
     * PATTERN
     */
    schema = MODULE_CREATE_YANG("TPATTERN_0", "leaf port {type string"
            "{pattern '[a-zA-Z_][a-zA-Z0-9\\-_.]*';}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[a-zA-Z_][a-zA-Z0-9\\-_.]*", NULL, NULL, NULL, 0, NULL);

    schema = MODULE_CREATE_YANG("TPATTERN_1", "leaf port {type string{"
            "   pattern '[a-zA-Z_][a-zA-Z0-9\\-_.]*' ;"
            "   pattern 'abc.*' {modifier invert-match;}"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "abc.*", 0, 0x1, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 2, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[a-zA-Z_][a-zA-Z0-9\\-_.]*", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[1]), "\x15" "abc.*", NULL, NULL, NULL, 0, NULL);

    schema = MODULE_CREATE_YANG("TPATTERN_2",
            "typedef my_type {"
            "   type string{"
            "       pattern '[a-zA-Z_][a-zA-Z0-9\\-_.]*' ;"
            "       pattern 'abc.*' {modifier invert-match;}"
            "}}"
            "leaf port {type my_type {pattern 'bcd.*';}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 3);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "abc.*", 0, 0x1, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[2];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "bcd.*", 0, 0x0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "bcd.*", NULL, NULL, NULL, 0, NULL);

    /*
     * TEST pattern error
     */
    schema = MODULE_CREATE_YANG("TPATTERN_ERR_0", "leaf port {type string {"
            "pattern '[a-zA-Z_[a-zA-Z0-9\\-_.*';"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Regular expression \"[a-zA-Z_[a-zA-Z0-9\\-_.*\" is not valid (\"\": missing terminating ] for character class).",
            "/TPATTERN_ERR_0:port", 0);

    schema = MODULE_CREATE_YANG("TDEFAULT_0",
            "typedef my_type {"
            "   type string{"
            "       pattern \"[a-zA-Z_][a-zA-Z0-9\\\\-_.]*\";"
            "       length  \"2 .. 5 | 10\";"
            "   }"
            "   default \"a1i-j\";"
            "}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, "a1i-j");
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 2);
    assert_int_equal(range->parts[0].max_u64, 5);
    assert_int_equal(range->parts[1].min_u64, 10);
    assert_int_equal(range->parts[1].max_u64, 10);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /* TEST pattern backslash
     * The '[' character is escaped, thus character group is broken.
     */

    schema = MODULE_CREATE_YANG("TPATTERN_BC_ERR_1", "leaf port {type string {"
            "pattern '\\[a]b';" /* pattern '\[a]b'; */
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Regular expression \"\\[a]b\" is not valid (\"]b\": character group doesn't begin with '[').",
            "/TPATTERN_BC_ERR_1:port", 0);

    schema = MODULE_CREATE_YANG("TPATTERN_BC_ERR_2", "leaf port {type string {"
            "pattern \"\\\\[a]b\";" /* pattern "\\[a]b"; */
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Regular expression \"\\[a]b\" is not valid (\"]b\": character group doesn't begin with '[').",
            "/TPATTERN_BC_ERR_2:port", 0);

    /* PATTERN AND LENGTH */
    schema = MODULE_CREATE_YANG("TPL_0",
            "typedef my_type {"
            "   type string{"
            "       length  \"2 .. 10\";"
            "   }"
            "}"
            "leaf port {type my_type{ pattern \"[a-zA-Z_][a-zA-Z0-9\\\\-_.]*\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_u64, 2);
    assert_int_equal(range->parts[0].max_u64, 10);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[a-zA-Z_][a-zA-Z0-9\\-_.]*",
            NULL, NULL, NULL, 0, NULL);
}

static void
test_schema_yin(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_pattern *pattern;
    struct lysc_range *range;

    /* TEST BASE STRING */
    schema = MODULE_CREATE_YIN("base", "<leaf name=\"port\"> <type name=\"string\"/> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_STRING, 0, 0);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "string", 0, 0, 1, 0, 0, 0);

    /* TEST MODULE T0 */
    schema = MODULE_CREATE_YIN("T0", "<leaf name=\"port\"> <type name=\"string\">"
            "<length value=\"10 .. max\"/>"
            "</type> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_true(range->parts[0].max_u64 == 18446744073709551615ull);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "10 .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T1 */
    schema = MODULE_CREATE_YIN("T1", "<leaf name=\"port\"> <type name=\"string\">"
            "  <length value=\"min .. 20 | 50\"/>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 0);
    assert_int_equal(range->parts[0].max_u64, 20);
    assert_int_equal(range->parts[1].min_u64, 50);
    assert_int_equal(range->parts[1].max_u64, 50);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "min .. 20 | 50", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T2 */
    schema = MODULE_CREATE_YIN("T2", "<leaf name=\"port\"> <type name=\"string\">"
            "<length value=\"10 .. 20 | 50 .. 100 | 255\"/>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_int_equal(range->parts[0].max_u64, 20);
    assert_int_equal(range->parts[1].min_u64, 50);
    assert_int_equal(range->parts[1].max_u64, 100);
    assert_int_equal(range->parts[2].min_u64, 255);
    assert_int_equal(range->parts[2].max_u64, 255);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "string", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "10 .. 20 | 50 .. 100 | 255", NULL, NULL, NULL, 0, NULL);

    /* SUBTYPE MODULE T2 */
    schema = MODULE_CREATE_YIN("TS0",
            "<typedef name=\"my_type\">"
            "    <type name=\"string\"> <length value=\"10 .. 20 | 50 .. 100 | 255\"/> </type>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "    <length value=\"min .. 15 | max\"/>"
            "</type> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 0);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 10);
    assert_int_equal(range->parts[0].max_u64, 15);
    assert_int_equal(range->parts[1].min_u64, 255);
    assert_int_equal(range->parts[1].max_u64, 255);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x10, 0, 1, "my_type", 0, 0, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.length, "min .. 15 | max", NULL, NULL, NULL, 0, NULL);

    /* ERROR TESTS NEGATIVE VALUE */
    schema = MODULE_CREATE_YIN("ERR0", "<leaf name=\"port\"> <type name=\"string\">"
            "<length value =\"-1 .. 20\"/> </type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid length restriction - value \"-1\" does not fit the type limitations.", "/ERR0:port", 0);

    schema = MODULE_CREATE_YIN("ERR1", "<leaf name=\"port\"> <type name=\"string\">"
            "<length value=\"100 .. 18446744073709551616\"/>"
            "</type> </leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid length restriction - invalid value \"18446744073709551616\".", "/ERR1:port", 0);

    schema = MODULE_CREATE_YIN("ERR2", "<leaf name=\"port\">"
            "<type name=\"string\"> <length value=\"10 .. 20 | 20 .. 30\"/>"
            "</type> </leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EEXIST);
    CHECK_LOG_CTX("Invalid length restriction - values are not in ascending order (20).", "/ERR2:port", 0);

    schema = MODULE_CREATE_YIN("ERR3",
            "<typedef name=\"my_type\"> <type name=\"string\"/> </typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"> <length value=\"-1 .. 15\"/>"
            "</type> </leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid length restriction - value \"-1\" does not fit the type limitations.", "/ERR3:port", 0);

    /*
     * PATTERN
     */
    schema = MODULE_CREATE_YIN("TPATTERN_0", "<leaf name=\"port\"> <type name=\"string\">"
            "<pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>"
            "</type> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[a-zA-Z_][a-zA-Z0-9\\-_.]*", NULL, NULL, NULL, 0, NULL);

    schema = MODULE_CREATE_YIN("TPATTERN_1", "<leaf name=\"port\"> <type name=\"string\">"
            "   <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>"
            "   <pattern value=\"abc.*\"> <modifier value=\"invert-match\"/> </pattern>"
            "</type> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "abc.*", 0, 0x1, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 2, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[a-zA-Z_][a-zA-Z0-9\\-_.]*", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[1]), "\x15" "abc.*", NULL, NULL, NULL, 0, NULL);

    schema = MODULE_CREATE_YIN("TPATTERN_2",
            "<typedef name=\"my_type\">"
            "   <type name=\"string\">"
            "       <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>"
            "       <pattern value=\"abc.*\"> <modifier value=\"invert-match\"/> </pattern>"
            "</type> </typedef>"
            "<leaf name=\"port\"><type name=\"my_type\"> <pattern value=\"bcd.*\"/> </type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 3);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "abc.*", 0, 0x1, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[2];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "bcd.*", 0, 0x0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "bcd.*", NULL, NULL, NULL, 0, NULL);

    /*
     * TEST pattern error
     * */
    schema = MODULE_CREATE_YIN("TPATTERN_ERR_0",
            "<leaf name=\"port\"> <type name=\"string\">"
            "   <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.*\"/>"
            "</type> </leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Regular expression \"[a-zA-Z_][a-zA-Z0-9\\-_.*\" is not valid (\"\": missing terminating ] for character class).",
            "/TPATTERN_ERR_0:port", 0);

    /*
     * DEFAUT VALUE
     */
    schema = MODULE_CREATE_YIN("TDEFAULT_0",
            "<typedef name=\"my_type\">"
            "   <type name=\"string\">"
            "       <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>"
            "       <length  value=\"2 .. 5 | 10\"/>"
            "   </type>"
            "   <default value=\"a1i-j\"/>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"/> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, "a1i-j");
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 1, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[a-zA-Z_][a-zA-Z0-9\\-_.]*", 0, 0, NULL);
    range = ((struct lysc_type_str *)lysc_leaf->type)->length;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 2);
    assert_int_equal(range->parts[0].max_u64, 5);
    assert_int_equal(range->parts[1].min_u64, 10);
    assert_int_equal(range->parts[1].max_u64, 10);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("TDEFAULT_1",
            "<typedef name=\"my_type\">"
            "   <type name=\"string\">"
            "   </type>"
            "   <default value=\"a1i-j&lt;\"/>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"/> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, "a1i-j<");
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 0);
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_u64, 2);
    assert_int_equal(range->parts[0].max_u64, 5);
    assert_int_equal(range->parts[1].min_u64, 10);
    assert_int_equal(range->parts[1].max_u64, 10);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("TDEFAULT_2",
            "<typedef name=\"my_type\">"
            "   <type name=\"string\">"
            "       <length  value=\"2\"/>"
            "   </type>"
            "   <default value=\"a1i-j&lt;\"/>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"/> </leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied length - string \"a1i-j<\" length is not allowed.).",
            "/TDEFAULT_2:port", 0);

    schema = MODULE_CREATE_YIN("TDEFAULT_3",
            "<typedef name=\"my_type\">"
            "   <default value=\"a1i-j&lt;\"/>"
            "   <type name=\"string\">"
            "</type> </typedef>"
            "<leaf name=\"port\"><type name=\"my_type\"> <pattern value=\"bcd.*\"/> </type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied pattern - \"a1i-j<\" does not conform to \"bcd.*\".).",
            "/TDEFAULT_3:port", 0);

}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0",
            "leaf port {type string {"
            "length \"min .. 20 | 50\";"
            "pattern \"p.*\\\\\\\\\";"
            "pattern 'p4.*' {modifier invert-match;}"
            "}default \"p\\\"<\\\\\";}");
    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "  <leaf name=\"port\">\n"
            "    <type name=\"string\">\n"
            "      <length value=\"min .. 20 | 50\"/>\n"
            "      <pattern value=\"p.*\\\\\"/>\n"
            "      <pattern value=\"p4.*\">\n"
            "        <modifier value=\"invert-match\"/>\n"
            "      </pattern>\n"
            "    </type>\n"
            "    <default value=\"p&quot;&lt;\\\"/>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT1",
            "\n"
            "  leaf port {\n"
            "    type string {\n"
            "      length \"min .. 20 | 50\";\n"
            "      pattern \"p.*\\\\\\\\\";\n"
            "      pattern \"p4.*\" {\n"
            "        modifier invert-match;\n"
            "      }\n"
            "    }\n"
            "    default \"p\\\"<\\\\\";\n"
            "  }\n");
    schema_yin = MODULE_CREATE_YIN("PRINT1",
            "  <leaf name=\"port\">\n"
            "    <type name=\"string\">\n"
            "      <length value=\"min .. 20 | 50\"/>\n"
            "      <pattern value=\"p.*\\\\\"/>\n"
            "      <pattern value=\"p4.*\">\n"
            "        <modifier value=\"invert-match\"/>\n"
            "      </pattern>\n"
            "    </type>\n"
            "    <default value=\"p&quot;&lt;\\\"/>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yin, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, schema_yang);
    free(printed);
}

static void
test_data_xml(void **state)
{
    const char *schema;
    struct lyd_node *tree;
    const char *data;
    struct lysc_node_container *lysc_root;
    struct lyd_node_inner *lyd_root;

    /* NO RESTRICTION TESTS */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    /* space on start and new line */
    TEST_SUCCESS_XML("T0", " 50  \n\t 64", STRING, " 50  \n\t 64");
    /* nuber as string */
    TEST_SUCCESS_XML("T0", "50", STRING, "50");
    TEST_SUCCESS_XML("T0", "+250", STRING, "+250");
    /* references */
    TEST_SUCCESS_XML("T0", "&quot;", STRING, "\"");
    TEST_SUCCESS_XML("T0", "|&amp;|", STRING, "|&|");
    TEST_SUCCESS_XML("T0", "&apos;", STRING, "'");
    TEST_SUCCESS_XML("T0", "&lt;", STRING, "<");
    TEST_SUCCESS_XML("T0", "&gt;", STRING, ">");
    TEST_SUCCESS_XML("T0", "&#x42f;&#1071;", STRING, "ЯЯ");
    /* special characters */
    TEST_SUCCESS_XML("T0", "\"", STRING, "\"");
    TEST_SUCCESS_XML("T0", "'", STRING, "'");
    TEST_SUCCESS_XML("T0", ">", STRING, ">");
    TEST_SUCCESS_XML("T0", "", STRING, "");
    TEST_SUCCESS_XML("T0", "&amp;&lt;lt;", STRING, "&<lt;");
    /* CDATA IS NOT SUPPORTED
     * TEST_SUCCESS_XML("T2", "<![CDATA[<greeting>Hello, world! & Wecome</greeting>]]>", STRING,
     *                        "<greeting>Hello, world! & Wecome</greeting>");
     *  COMMENT IN MIDDLE OF TEXT IS NOT SUPPORTED
     *  TEST_SUCCESS_XML("T2", "this isn't <!--' this is comment '-->comment",
     *                 STRING, "this isn't comment");
     */

    /* error */
    TEST_ERROR_XML("T0", "< df");
    CHECK_LOG_CTX("Child element \"df\" inside a terminal node \"port\" found.", "/T0:port", 1);
    TEST_ERROR_XML("T0", "&text;");
    CHECK_LOG_CTX("Entity reference \"&text;</po\" not supported, only predefined references allowed.", NULL, 1);
    TEST_ERROR_XML("T0", "\"&#x8;\"");
    CHECK_LOG_CTX("Invalid character reference \"&#x8;\"</port\" (0x00000008).", NULL, 1);

    /* TEST INVERTED PATTERN ADN LENGTH */
    schema = MODULE_CREATE_YANG("T1", "leaf port {type string {"
            "       length  \"5 .. 10 | 20\";"
            "       pattern '[a-zA-Z_][a-zA-Z0-9\\-_.<]*' ;"
            "       pattern 'p4.*' {modifier invert-match;"
            "           error-app-tag \"pattern-violation\"; error-message \"invalid pattern of value\";}"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    /* inverted value */
    TEST_SUCCESS_XML("T1", "abcde",    STRING, "abcde");
    TEST_SUCCESS_XML("T1", "abcde&lt;", STRING, "abcde<");
    TEST_ERROR_XML("T1", "p4abc");
    CHECK_LOG_CTX_APPTAG("invalid pattern of value", "/T1:port", 1, "pattern-violation");
    /* size 20 */
    TEST_SUCCESS_XML("T1", "ahojahojahojahojahoj", STRING, "ahojahojahojahojahoj");
    TEST_SUCCESS_XML("T1", "abc-de", STRING, "abc-de");
    /* ERROR LENGTH  */
    TEST_ERROR_XML("T1", "p4a&lt;");
    CHECK_LOG_CTX("Unsatisfied length - string \"p4a<\" length is not allowed.", "/T1:port", 1);

    /* TEST DEFAULT VALUE */
    schema = MODULE_CREATE_YANG("T2",
            "container cont {\n"
            "    leaf port {type string {length \"0 .. 50 | 105\";} default \"test\";}"
            "}");
    /* using default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "<cont xmlns=\"urn:tests:" "T2" "\">"  "</cont>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *) tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *) lyd_root->child, 1, 0, 0, 1, 1,
            STRING, "test");\
    lyd_free_all(tree);

    /* rewriting default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "<cont xmlns=\"urn:tests:T2\">" "<port> 52 </port>" "</cont>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *) tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *) lyd_root->child, 0, 0, 0, 1, 1,
            STRING, " 52 ");
    lyd_free_all(tree);

    /* WHIT STRING TEST */
    schema = MODULE_CREATE_YANG("T_WHITE", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_XML("T_WHITE", " \n\t", STRING, " \n\t");
    TEST_SUCCESS_XML("T_WHITE", " \n&lt;\t", STRING, " \n<\t");

    /* UTF-8 length and pattern*/
    schema = MODULE_CREATE_YANG("T_UTF8", "leaf port {type string {"
            "       length  \"5 .. 10\";"
            "       pattern '[€]{5,7}' ;"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_XML("T_UTF8", "€€€€€",    STRING, "€€€€€");
    TEST_ERROR_XML("T_UTF8", "€€€");
    CHECK_LOG_CTX("Unsatisfied length - string \"€€€\" length is not allowed.", "/T_UTF8:port", 1);
    TEST_ERROR_XML("T_UTF8", "€€€€€€€€");
    CHECK_LOG_CTX("Unsatisfied pattern - \"€€€€€€€€\" does not conform to \"[€]{5,7}\".", "/T_UTF8:port", 1);

    /* ANCHOR TEST ^$ is implicit */
    schema = MODULE_CREATE_YANG("T_ANCHOR", "leaf port {type string {"
            "       pattern 'a.*b' ;"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_ERROR_XML("T_ANCHOR", "abc");
    CHECK_LOG_CTX("Unsatisfied pattern - \"abc\" does not conform to \"a.*b\".", "/T_ANCHOR:port", 1);
    TEST_ERROR_XML("T_ANCHOR", "cab");
    CHECK_LOG_CTX("Unsatisfied pattern - \"cab\" does not conform to \"a.*b\".", "/T_ANCHOR:port", 1);
}

static void
test_data_json(void **state)
{
    const char *schema;
    struct lyd_node *tree;
    const char *data;
    struct lysc_node_container *lysc_root;
    struct lyd_node_inner *lyd_root;

    /* NO RESTRICTION TESTS */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_JSON("T0", "this is text", STRING, "this is text");
    /* space on start and new line */
    TEST_SUCCESS_JSON("T0", " 50  \\n\\t 64", STRING, " 50  \n\t 64");
    /* nuber as string */
    TEST_SUCCESS_JSON("T0", "50", STRING, "50");
    TEST_SUCCESS_JSON("T0", "+250", STRING, "+250");
    /* references */
    TEST_SUCCESS_JSON("T0", "\\\"", STRING, "\"");
    TEST_SUCCESS_JSON("T0", "|&|", STRING, "|&|");
    TEST_SUCCESS_JSON("T0", "<", STRING, "<");
    TEST_SUCCESS_JSON("T0", ">", STRING, ">");
    TEST_SUCCESS_JSON("T0", "\\u042F", STRING, "Я");
    TEST_SUCCESS_JSON("T0", "\\u042FFF", STRING, "ЯFF");
    /* special characters */
    TEST_SUCCESS_JSON("T0", "'", STRING, "'");
    TEST_SUCCESS_JSON("T0", ">", STRING, ">");
    TEST_SUCCESS_JSON("T0", "", STRING, "");
    TEST_SUCCESS_JSON("T0", "\\\" \\\\ \\r \\/ \\n \\t \\u20ac", STRING, "\" \\ \r / \n \t €");

    /* ERROR invalid json string */
    TEST_ERROR_JSON("T0", "\n");
    CHECK_LOG_CTX("Invalid character in JSON string \"\n\" (0x0000000a).", NULL, 1);
    /* backspace and form feed are valid JSON escape sequences, but the control characters they represents are not allowed values for YANG string type */
    TEST_ERROR_JSON("T0", "\\b");
    CHECK_LOG_CTX("Invalid character reference \"\\b\" (0x00000008).", NULL, 1);

    TEST_ERROR_JSON("T0", "\\f");
    CHECK_LOG_CTX("Invalid character reference \"\\f\" (0x0000000c).", NULL, 1);

    TEST_ERROR_JSON("T0", "\"");
    CHECK_LOG_CTX("Invalid character sequence \"\"}\", expected a JSON object-end or next item.", NULL, 1);

    TEST_ERROR_JSON("T0", "aabb \\x");
    CHECK_LOG_CTX("Invalid character escape sequence \\x.", NULL, 1);

    /* TEST INVERTED PATTERN ADN LENGTH */
    schema = MODULE_CREATE_YANG("T1", "leaf port {type string {"
            "       length  \"5 .. 10 | 20\";"
            "       pattern '[a-zA-Z_][a-zA-Z0-9\\-_.<]*\\n[a-zA-Z0-9\\-_.<]*' ;"
            "       pattern 'p4.*\\n' {modifier invert-match;}"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    /* inverted value */
    TEST_SUCCESS_JSON("T1", "a\\nbcde",     STRING, "a\nbcde");
    TEST_ERROR_JSON("T1", "p4abc\\n");
    CHECK_LOG_CTX("Unsatisfied pattern - \"p4abc\n\" does not conform to inverted \"p4.*\\n\".", "/T1:port", 1);
    /* size 20 */
    TEST_SUCCESS_JSON("T1", "ahojahojaho\\njahojaho", STRING, "ahojahojaho\njahojaho");
    TEST_SUCCESS_JSON("T1", "abc\\n-de", STRING, "abc\n-de");
    /* ERROR LENGTH  */
    TEST_ERROR_JSON("T1", "p4a\u042F");
    CHECK_LOG_CTX("Unsatisfied length - string \"p4aЯ\" length is not allowed.", "/T1:port", 1);

    /* TEST DEFAULT VALUE */
    schema = MODULE_CREATE_YANG("T_DEFAULT2",
            "container cont {\n"
            "    leaf port {type string {length \"0 .. 50 | 105\";} default \"test\";}"
            "}");
    /* using default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "{\"T_DEFAULT2:cont\":{}}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *) tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *) lyd_root->child, 1, 0, 0, 1, 1,
            STRING, "test");\
    lyd_free_all(tree);

    /* rewriting default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "{\"T_DEFAULT2:cont\":{\":port\": \" 52 \"}}";\
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *) tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *) lyd_root->child, 0, 0, 0, 1, 1,
            STRING, " 52 ");
    lyd_free_all(tree);

    /* UTF-8 length and pattern*/
    schema = MODULE_CREATE_YANG("T_UTF8", "leaf port {type string {"
            "       length  \"5 .. 10\";"
            "       pattern '[€]{5,7}' ;"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_JSON("T_UTF8", "€€€€€",    STRING, "€€€€€");
    TEST_ERROR_JSON("T_UTF8", "€€€");
    CHECK_LOG_CTX("Unsatisfied length - string \"€€€\" length is not allowed.", "/T_UTF8:port", 1);
    TEST_ERROR_JSON("T_UTF8", "€€€€€€€€");
    CHECK_LOG_CTX("Unsatisfied pattern - \"€€€€€€€€\" does not conform to \"[€]{5,7}\".", "/T_UTF8:port", 1);

    /* ANCHOR TEST ^$ is implicit */
    schema = MODULE_CREATE_YANG("T_ANCHOR", "leaf port {type string {"
            "       pattern 'a.*b' ;"
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_ERROR_JSON("T_ANCHOR", "abc");
    CHECK_LOG_CTX("Unsatisfied pattern - \"abc\" does not conform to \"a.*b\".", "/T_ANCHOR:port", 1);
    TEST_ERROR_JSON("T_ANCHOR", "cb");
    CHECK_LOG_CTX("Unsatisfied pattern - \"cb\" does not conform to \"a.*b\".", "/T_ANCHOR:port", 1);
}

static void
test_data_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "port", "");
    TEST_SUCCESS_LYB("lyb", "port", "a");
    TEST_SUCCESS_LYB("lyb", "port", "abcdefghijklmnopqrstuvwxyz");
}

static void
test_diff(void **state)
{
    (void) state;
    const char *schema;

    schema = MODULE_CREATE_YANG("T_DIFF", "leaf port {type string {length \"6 .. 50 | 120\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    struct lyd_node *model_1, *model_2;
    struct lyd_node *diff;
    const char *data_1 = "<port xmlns=\"urn:tests:T_DIFF\"> text abc &lt; </port>";
    const char *data_2 = "<port xmlns=\"urn:tests:T_DIFF\"> text abc &gt; </port>";
    const char *diff_expected = "<port xmlns=\"urn:tests:T_DIFF\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"replace\" yang:orig-default=\"false\""
            " yang:orig-value=\" text abc &lt; \"> text abc &gt; </port>";

    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* create data from diff */
    diff_expected = "<port xmlns=\"urn:tests:T_DIFF\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"replace\" yang:orig-default=\"false\""
            " yang:orig-value=\" 10^20 \">jjjjjjj</port>";

    CHECK_PARSE_LYD_PARAM(diff_expected, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, diff);
    data_1 = "<port xmlns=\"urn:tests:T_DIFF\"> 10^20 </port>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    const char *expected = "<port xmlns=\"urn:tests:T_DIFF\">jjjjjjj</port>";

    CHECK_LYD_STRING_PARAM(model_1, expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_free_all(model_1);
    lyd_free_all(diff);

    /* check creating data breaking restrictions */
    diff_expected = "<port xmlns=\"urn:tests:T_DIFF\" "
            "xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\" 555 555\">"
            "121</port>";
    CHECK_PARSE_LYD_PARAM(diff_expected, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, model_1);
    CHECK_LOG_CTX("Unsatisfied length - string \"121\" length is not allowed.", "/T_DIFF:port", 1);

    /* diff from default value */
    data_1 = "<cont xmlns=\"urn:tests:T_DIFF1\"></cont>";
    data_2 = "<cont xmlns=\"urn:tests:T_DIFF1\"> <port> 6 </port> </cont>";
    diff_expected = "<cont xmlns=\"urn:tests:T_DIFF1\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"create\"><port> 6 </port></cont>";

    schema = MODULE_CREATE_YANG("T_DIFF1",
            "container cont {\n"
            "    leaf port {type string; default \" 20\n30 \";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(diff);

    lyd_free_all(model_1);
    lyd_free_all(model_2);
}

static void
test_print(void **state)
{
    (void) state;
    const char *schema;

    schema = MODULE_CREATE_YANG("T_PRINT", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    struct lyd_node *model_1;
    const char *data_1 = "<port xmlns=\"urn:tests:T_PRINT\"> &lt; hello &gt;  </port>";

    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);

    /* XML */
    const char *expected_xml = "<port xmlns=\"urn:tests:T_PRINT\"> &lt; hello &gt;  </port>";

    CHECK_LYD_STRING_PARAM(model_1, expected_xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    /* JSON */
    const char *expected_json = "{\"T_PRINT:port\":\" < hello >  \"}";

    CHECK_LYD_STRING_PARAM(model_1, expected_json, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    lyd_free_all(model_1);
}

static void
test_plugin_store(void **state)
{
    (void) state;

    const char *val_text = NULL;
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value value = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);
    struct lysc_type *lysc_type;
    char *alloc_text;
    unsigned int alloc_text_size;
    LY_ERR ly_ret;
    const char *schema;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type string {length \"0 .. 10\";"
            "pattern '[0-9\\n<>\\\"\\|]*' ;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *) mod->compiled->data)->type;

    /* check proper type */
    assert_string_equal("libyang 2 - string, version 1", type->id);

    /* check store */
    val_text = "20";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "20");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "150\n";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "150\n");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "<\"150>\n";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "<\"150>\n");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "<\"150>\n|hi how are you";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, 8,
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "<\"150>\n|");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /*
     * minor tests
     * dynamic alocated input text
     */
    val_text = "<250>";
    alloc_text_size = strlen(val_text);
    alloc_text = (char *) malloc(alloc_text_size + 1);
    memcpy(alloc_text, val_text, alloc_text_size + 1);

    ly_ret = type->store(UTEST_LYCTX, lysc_type, alloc_text, alloc_text_size,
            LYPLG_TYPE_STORE_DYNAMIC, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    alloc_text = NULL;
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, STRING, "<250>");
    type->free(UTEST_LYCTX, &value);

    /* wrong lysc_type of value */
    struct lysc_type lysc_type_test = *lysc_type;

    lysc_type_test.basetype = LY_TYPE_UINT8;
    val_text = "20";
    ly_ret = type->store(UTEST_LYCTX, &lysc_type_test, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EINVAL, ly_ret);
    ly_err_free(err);

    /* TEST pattern backslash */

    schema = MODULE_CREATE_YANG("TPATTERN_BC_1", "leaf port {type string {"
            "pattern '\\\\[a]b';"  /* pattern '\\[a]b'; */
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    val_text = "\\ab";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "\\ab");
    type->free(UTEST_LYCTX, &value);

    schema = MODULE_CREATE_YANG("TPATTERN_BC_2", "leaf port {type string {"
            "pattern \"\\\\\\\\[a]b\";" /* pattern "\\\\[a]b"; */
            "}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    val_text = "\\ab";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, STRING, "\\ab");
    type->free(UTEST_LYCTX, &value);

    /* ERROR TESTS */

    val_text = "10 \"| bcdei";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "012345678901";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "10";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    /* LYPLG_TYPE_STORE_ONLY test */
    val_text = "10 \"| bcdei";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            LYPLG_TYPE_STORE_ONLY, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL,
            &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    type->free(UTEST_LYCTX, &value);
    ly_err_free(err);
}

static void
test_plugin_compare(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;

    /* different type */
    const char *diff_type_text = "20";
    struct lyd_value  diff_type_val;
    struct lysc_type *diff_type;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "typedef my_int_type {type string; }"
            "leaf p1 {type my_int_type;}"
            "leaf p2 {type my_int_type;}"
            "leaf p3 {type string;}"
            "leaf p4 {type uint8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *) mod->compiled->data)->type;

    /* CREATE VALUES */
    const char *val_init[] = {"hi", "hello", "hi", "hello", "hell", "hh"};

    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /*  BASIC TEST; */
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[0]), &(values[0])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[0]), &(values[2])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[0]), &(values[1])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[2])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[1]), &(values[3])));

    /* SAME TYPE but different node */
    diff_type_text = "hi";
    diff_type = ((struct lysc_node_leaf *) mod->compiled->data->next)->type;
    ly_ret = diff_type->plugin->store(UTEST_LYCTX, diff_type, diff_type_text, strlen(diff_type_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &diff_type_val, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &diff_type_val, &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[1])));
    type->free(UTEST_LYCTX, &(diff_type_val));

    /* delete values */
    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_print(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;

    /* create schema. Prepare common used variables */
    const char *schema = MODULE_CREATE_YANG("defs", "leaf port {type string;}");

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *) mod->compiled->data)->type;

    /* CREATE VALUES */
    const char *val_init[] = {"20", "0x4A", "<|>", "\""};

    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print value */
    ly_bool dynamic = 0;

    assert_string_equal("20", type->print(UTEST_LYCTX, &(values[0]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("0x4A", type->print(UTEST_LYCTX, &(values[1]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("<|>", type->print(UTEST_LYCTX, &(values[2]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("\"", type->print(UTEST_LYCTX, &(values[3]), LY_VALUE_XML, NULL, &dynamic, NULL));

    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_dup(void **state)
{

    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);
    struct lysc_type *lysc_type[2];
    const char *schema;
    LY_ERR ly_ret;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type[0] = ((struct lysc_node_leaf *) mod->compiled->data)->type;

    schema = MODULE_CREATE_YANG("T1",
            "typedef my_int_type {"
            "    type string {length \"1 .. 100\";} default 20;"
            "}"
            "leaf port {type my_int_type; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type[1] = ((struct lysc_node_leaf *) mod->compiled->data)->type;

    /* CREATE VALUES */
    const char *val_init[] = {"20", "0x4A", "<\">", "0x4A"};

    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type[it % 2], val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print duplicate value */
    struct lyd_value dup_value;

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[0]), &dup_value));
    CHECK_LYD_VALUE(dup_value, STRING, "20");
    assert_ptr_equal(dup_value.realtype, values[0].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[1]), &dup_value));
    CHECK_LYD_VALUE(dup_value, STRING, "0x4A");
    assert_ptr_equal(dup_value.realtype, values[1].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[2]), &dup_value));
    CHECK_LYD_VALUE(dup_value, STRING, "<\">");
    assert_ptr_equal(dup_value.realtype, values[2].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[3]), &dup_value));
    CHECK_LYD_VALUE(dup_value, STRING, "0x4A");
    assert_ptr_equal(dup_value.realtype, values[3].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    /* error tests */
    assert_int_equal(LY_EINVAL, type->duplicate(NULL, &(values[0]), &dup_value));

    for (int unsigned it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema_yang),
        UTEST(test_schema_yin),
        UTEST(test_schema_print),
        UTEST(test_data_xml),
        UTEST(test_data_json),
        UTEST(test_data_lyb),
        UTEST(test_diff),
        UTEST(test_print),

        UTEST(test_plugin_store),
        UTEST(test_plugin_compare),
        UTEST(test_plugin_print),
        UTEST(test_plugin_dup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
