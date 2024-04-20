/**
 * @file int8.c
 * @author Radek Iša <isa@cesnet.cz>
 * @brief test for int8 values
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* INCLUDE UTEST HEADER */
#define  _UTEST_MAIN_
#include "../utests.h"

/* GLOBAL INCLUDE HEADERS */
#include <ctype.h>

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "path.h"
#include "plugins_internal.h"

#define LYD_TREE_CREATE(INPUT, MODEL) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, MODEL)

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

#define TEST_SUCCESS_XML(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_SUCCESS_JSON(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "{\"" MOD_NAME ":port\":" DATA "}"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
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

#define TEST_ERROR_XML(MOD_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

#define TEST_ERROR_JSON(MOD_NAME, DATA) \
    { \
        struct lyd_node *tree; \
        const char *data = "{\"" MOD_NAME ":port\":" DATA "}"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_schema_yang(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_range *range;

    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"0 .. 50 | 127\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 50);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 127", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T0 */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type int8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST MODULE T1 */
    schema = MODULE_CREATE_YANG("T1", "leaf port {type int8 {range \"0 .. 50 |51 .. 60\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 50);
    assert_int_equal(range->parts[1].min_64, 51);
    assert_int_equal(range->parts[1].max_64, 60);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 |51 .. 60", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T1 */
    schema = MODULE_CREATE_YANG("T2", "leaf port {type int8 {range \"20\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, 20);
    assert_int_equal(range->parts[0].max_64, 20);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "20", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T3 */
    schema = MODULE_CREATE_YANG("T3", "leaf port {type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, -60);
    assert_int_equal(range->parts[1].min_64, -1);
    assert_int_equal(range->parts[1].max_64,  1);
    assert_int_equal(range->parts[2].min_64, 60);
    assert_int_equal(range->parts[2].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "-128 .. -60 | -1 .. 1 |  60 .. 127", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T4 */
    schema = MODULE_CREATE_YANG("T4", "leaf port {type int8 {range \"1 .. 1\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, 1);
    assert_int_equal(range->parts[0].max_64, 1);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "1 .. 1", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T4 */
    schema = MODULE_CREATE_YANG("T5", "leaf port {type int8 {range \"7\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, 7);
    assert_int_equal(range->parts[0].max_64, 7);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "7", NULL, NULL, NULL, 0, NULL);

    /* TEST MODULE T4 */
    schema = MODULE_CREATE_YANG("T6", "leaf port {type int8 {range \"min .. max\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST ERROR -60 .. 0 | 0 .. 127 */
    schema = MODULE_CREATE_YANG("ERR0", "leaf port {type int8 {range \"-60 .. 0 | 0 .. 127\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EEXIST);
    CHECK_LOG_CTX("Invalid range restriction - values are not in ascending order (0).", "/ERR0:port", 0);

    /* TEST ERROR 0 .. 128 */
    schema = MODULE_CREATE_YANG("ERR1", "leaf port {type int8 {range \"0 .. 128\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"128\" does not fit the type limitations.", "/ERR1:port", 0);

    /* TEST ERROR -129 .. 126 */
    schema = MODULE_CREATE_YANG("ERR2", "leaf port {type int8 {range \"-129 .. 0\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"-129\" does not fit the type limitations.", "/ERR2:port", 0);

    /* TEST ERROR 0 */
    schema = MODULE_CREATE_YANG("ERR3", "leaf port {type int8 {range \"-129\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"-129\" does not fit the type limitations.", "/ERR3:port", 0);

    /*
     * TEST MODULE SUBTYPE
     */
    schema = MODULE_CREATE_YANG("TS0",
            "typedef my_int_type {"
            "    type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}"
            "}"
            "leaf my_leaf {type my_int_type; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, -60);
    assert_int_equal(range->parts[1].min_64, -1);
    assert_int_equal(range->parts[1].max_64,  1);
    assert_int_equal(range->parts[2].min_64, 60);
    assert_int_equal(range->parts[2].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_int_type", 0, 0, 1, 0, 0, 0);

    /* TEST SUBTYPE RANGE */
    schema = MODULE_CREATE_YANG("TS1",
            "typedef my_int_type {"
            "    type int8 {range \"-100 .. -60 | -1 .. 1 |  60 .. 127\";}"
            "}"
            "leaf my_leaf {type my_int_type {range \"min .. -60\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, -100);
    assert_int_equal(range->parts[0].max_64, -60);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_int_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. -60", NULL, NULL, NULL, 0, NULL);

    /* TEST SUBTYPE RANGE */
    schema = MODULE_CREATE_YANG("TS2",
            "typedef my_int_type {"
            "    type int8 {range \"-100 .. -60 | -1 .. 1 |  60 .. 120\";}"
            "}"
            "leaf my_leaf {type my_int_type {range \"70 .. max\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, 70);
    assert_int_equal(range->parts[0].max_64, 120);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_int_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "70 .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST SUBTYPE RANGE */
    schema = MODULE_CREATE_YANG("TS3",
            "typedef my_int_type {"
            "   type int8 {range \"-100 .. -60 | -1 .. 1 |  60 .. 127\";}"
            "}"
            "leaf my_leaf {type my_int_type {range \"-1 .. 1\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, -1);
    assert_int_equal(range->parts[0].max_64, 1);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_int_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "-1 .. 1", NULL, NULL, NULL, 0, NULL);

    /* TEST SUBTYPE RANGE */
    schema = MODULE_CREATE_YANG("TS4",
            "typedef my_int_type {"
            "   type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}"
            "}"
            "leaf my_leaf {type my_int_type { "
            "   range \"min .. -60 | -1 .. 1 |  60 .. max\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, -60);
    assert_int_equal(range->parts[1].min_64, -1);
    assert_int_equal(range->parts[1].max_64,  1);
    assert_int_equal(range->parts[2].min_64, 60);
    assert_int_equal(range->parts[2].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_int_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. -60 | -1 .. 1 |  60 .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST SUBTYPE ERROR min .. max */
    schema = MODULE_CREATE_YANG("TS_ERR0",
            "typedef my_int_type { type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}"
            "leaf my_leaf {type my_int_type {range \"min .. max\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (min .. max) is not equally or more limiting.",
            "/TS_ERR0:my_leaf", 0);

    /* TEST SUBTYPE ERROR -80 .. 80 */
    schema = MODULE_CREATE_YANG("TS_ERR1",
            "typedef my_int_type { type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}"
            " leaf my_leaf {type my_int_type {range \"-80 .. 80\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (-80 .. 80) is not equally or more limiting.",
            "/TS_ERR1:my_leaf", 0);

    /* TEST SUBTYPE ERROR 0 .. max */
    schema = MODULE_CREATE_YANG("TS_ERR2",
            "typedef my_int_type { type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}"
            "leaf my_leaf {type my_int_type {range \"0 .. max\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (0 .. max) is not equally or more limiting.",
            "/TS_ERR2:my_leaf", 0);

    /* TEST SUBTYPE ERROR -2 .. 2 */
    schema = MODULE_CREATE_YANG("TS_ERR3",
            "typedef my_int_type { type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}"
            "leaf my_leaf {type my_int_type {range \"-2 .. 2\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (-2 .. 2) is not equally or more limiting.",
            "/TS_ERR3:my_leaf", 0);

    /* TEST SUBTYPE ERROR -2 .. 2 */
    schema = MODULE_CREATE_YANG("TS_ERR4",
            "typedef my_int_type { type int8 {range \"-128 .. -60 | -1 .. 1 |  60 .. 127\";}}"
            "leaf my_leaf {type my_int_type {range \"-100 .. -90 | 100 .. 128\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"128\" does not fit the type limitations.",
            "/TS_ERR4:my_leaf", 0);

    /*
     * TEST DEFAULT VALUE
     */
    schema = MODULE_CREATE_YANG("DF0",
            "leaf port {"
            "    type int8 {range \"0 .. 50 | 127\";}"
            "    default \"20\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "20", 20);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 50);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "20");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 127", NULL, NULL, NULL, 0, NULL);

    /* TEST DEFAULT VALUE */
    schema = MODULE_CREATE_YANG("DF1", "leaf port {type int8 {range \"0 .. 50 | 127\";}"
            "default \"127\"; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "127", 127);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 50);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "127");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 127", NULL, NULL, NULL, 0, NULL);

    /* TEST DEFAULT VALUE ERROR */
    schema = MODULE_CREATE_YANG("TD_ERR0",
            "leaf port {"
            "   type int8 {range \"0 .. 50 | 127\";}"
            "   default \"128\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"128\" is out of type int8 min/max bounds.).",
            "/TD_ERR0:port", 0);

    /* TEST DEFAULT VALUE ERROR */
    schema = MODULE_CREATE_YANG("TD_ERR1",
            "leaf port {"
            "    type int8 {range \"0 .. 50 | 127\";}"
            "    default \"-1\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied range - value \"-1\" is out of the allowed range.).",
            "/TD_ERR1:port", 0);

    /* TEST DEFAULT VALUE ERROR */
    schema = MODULE_CREATE_YANG("TD_ERR2",
            "leaf port {"
            "    type int8 {range \"0 .. 50 | 127\";}"
            "    default \"60\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied range - value \"60\" is out of the allowed range.).",
            "/TD_ERR2:port", 0);

    /* TEST DEFAULT VALUE ERROR */
    schema = MODULE_CREATE_YANG("TD_ERR3",
            "typedef my_int_type { type int8 {range \"60 .. 127\";} default \"127\";}"
            "leaf my_leaf {type my_int_type {range \"70 .. 80\";}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied range - value \"127\" is out of the allowed range.).",
            "/TD_ERR3:my_leaf", 0);

    /* TEST DEFAULT HEXADECIMAL */
    schema = MODULE_CREATE_YANG("DF_HEX0",
            "leaf port {"
            "    type int8;"
            "    default \"0xf\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "15", 15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "0xf");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT HEXADECIMAL */
    schema = MODULE_CREATE_YANG("DF_HEX1",
            "leaf port {"
            "    type int8;"
            "    default \"-0xf\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "-15", -15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "-0xf");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT HEXADECIMAL */
    schema = MODULE_CREATE_YANG("DF_HEXI0",
            "leaf port {"
            "    type int8 {range \"0 .. 50 | 127\";}"
            "    default \"+0x7F\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "127", 127);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 50);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "+0x7F");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 127", NULL, NULL, NULL, 0, NULL);

    /* TEST DEFAULT HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YANG("DF_HEX2",
            "leaf port {"
            "    type int8;"
            "    default \"0xff\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0xff\" is out of type int8 min/max bounds.).",
            "/DF_HEX2:port", 0);

    /* TEST DEFAULT HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YANG("DF_HEX3",
            "leaf port {"
            "    type int8;"
            "    default \"-0x81\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"-0x81\" is out of type int8 min/max bounds.).",
            "/DF_HEX3:port", 0);

    /* TEST DEFAULT HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YANG("DF_HEX4",
            "leaf port {"
            "    type int8;"
            "    default \"0x80\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0x80\" is out of type int8 min/max bounds.).",
            "/DF_HEX4:port", 0);

    /* TEST DEFAULT VALUE OCTAL */
    schema = MODULE_CREATE_YANG("DF_OCT0",
            "leaf port {"
            "    type int8;"
            "    default \"017\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "15", 15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "017");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE OCTAL */
    schema = MODULE_CREATE_YANG("DF_OCT1",
            "leaf port {"
            "    type int8;"
            "    default \"-017\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "-15", -15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "-017");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE OCTAL */
    schema = MODULE_CREATE_YANG("DF_OCTI0",
            "leaf port {"
            "    type int8;"
            "    default \"+017\";"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "15", 15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "+017");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE OCTAL ERROR*/
    schema = MODULE_CREATE_YANG("DF_OCT2",
            "leaf port {"
            "    type int8;"
            "    default \"0377\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0377\" is out of type int8 min/max bounds.).",
            "/DF_OCT2:port", 0);

    /* TEST DEFAULT VALUE OCTAL ERROR*/
    schema = MODULE_CREATE_YANG("DF_OCT3",
            "leaf port {"
            "    type int8;"
            "    default \"-0201\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"-0201\" is out of type int8 min/max bounds.).",
            "/DF_OCT3:port", 0);

    /* TEST DEFAULT VALUE OCTAL ERROR*/
    schema = MODULE_CREATE_YANG("DF_OCT4",
            "leaf port {"
            "    type int8;"
            "    default \"0200\";"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0200\" is out of type int8 min/max bounds.).",
            "/DF_OCT4:port", 0);
}

static void
test_schema_yin(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_range *range;

    /* TEST T0 */
    schema = MODULE_CREATE_YIN("T0", "<leaf name=\"port\"> <type name=\"int8\"/> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST T1 */
    schema = MODULE_CREATE_YIN("T1",
            "<leaf name=\"port\"> "
            "    <type name=\"int8\"> <range value = \"0 .. 10\"/>  </type>"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    assert_int_equal(range->parts[0].min_64, 0);
    assert_int_equal(range->parts[0].max_64, 10);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 10", NULL, NULL, NULL, 0, NULL);

    /* TEST T1 */
    schema = MODULE_CREATE_YIN("T2",
            "<leaf name=\"port\"> "
            "    <type name=\"int8\"> <range value = \"-127 .. 10 | max\"/>  </type>"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, -127);
    assert_int_equal(range->parts[0].max_64, 10);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "-127 .. 10 | max", NULL, NULL, NULL, 0, NULL);

    /* TEST T2 */
    schema = MODULE_CREATE_YIN("T3",
            "<leaf name=\"port\"> "
            "    <type name=\"int8\"> <range value =\"min .. 10 | 11 .. 12 | 30\"/> </type>"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 3, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, 10);
    assert_int_equal(range->parts[1].min_64, 11);
    assert_int_equal(range->parts[1].max_64, 12);
    assert_int_equal(range->parts[2].min_64, 30);
    assert_int_equal(range->parts[2].max_64, 30);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. 10 | 11 .. 12 | 30", NULL, NULL, NULL, 0, NULL);

    /* TEST ERROR -60 .. 0 | 0 .. 127 */
    schema = MODULE_CREATE_YIN("TE0",
            "<leaf name=\"port\"> "
            "   <type name=\"int8\"> <range value = \"min .. 0 | 0 .. 12\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EEXIST);
    CHECK_LOG_CTX("Invalid range restriction - values are not in ascending order (0).", "/TE0:port", 0);

    /* TEST ERROR 0 .. 128 */
    schema = MODULE_CREATE_YIN("TE1",
            "<leaf name=\"port\">"
            "   <type name=\"int8\"> <range value = \"0 .. 128\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"128\" does not fit the type limitations.", "/TE1:port", 0);

    /* TEST ERROR -129 .. 126 */
    schema = MODULE_CREATE_YIN("TE2",
            "<leaf name=\"port\"> "
            "   <type name=\"int8\"> <range value =\"-129 .. 126\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EDENIED);
    CHECK_LOG_CTX("Invalid range restriction - value \"-129\" does not fit the type limitations.", "/TE2:port", 0);

    /* TEST YIN */
    schema = MODULE_CREATE_YIN("TS0",
            "<typedef name= \"my_int_type\">"
            "   <type name=\"int8\"> <range value = \"-127 .. 10 | max\"/>  </type>"
            "</typedef>"
            "<leaf name=\"my_leaf\"> <type name=\"my_int_type\"/> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "my_leaf", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, -127);
    assert_int_equal(range->parts[0].max_64, 10);
    assert_int_equal(range->parts[1].min_64, 127);
    assert_int_equal(range->parts[1].max_64, 127);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "my_leaf", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_int_type", 0, 0, 1, 0, 0, 0);

    /* TEST YIN */
    schema = MODULE_CREATE_YIN("TS1",
            "<typedef name= \"my_int_type\">"
            "    <type name=\"int8\"> <range value = \"-127 .. 10 | 90 .. 100\"/>  </type>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_int_type\"> <range value ="
            " \"min .. -30 | 100 .. max\"/>  </type> </leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, -127);
    assert_int_equal(range->parts[0].max_64, -30);
    assert_int_equal(range->parts[1].min_64, 100);
    assert_int_equal(range->parts[1].max_64, 100);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_int_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. -30 | 100 .. max", NULL, NULL, NULL, 0, NULL);

    /* TEST ERROR */
    schema = MODULE_CREATE_YIN("TS_ERR1",
            "<typedef name= \"my_int_type\">"
            "    <type name=\"int8\"> <range value = \"-127 .. 10 | 90 .. 100\"/>  </type>"
            "</typedef>"
            "<leaf name=\"port\">"
            "    <type name=\"my_int_type\"> <range value = \"min .. max\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (min .. max) is not equally or more limiting.",
            "/TS_ERR1:port", 0);

    /* TEST ERROR */
    schema = MODULE_CREATE_YIN("TS_ERR2",
            "<typedef name= \"my_int_type\">"
            "    <type name=\"int8\"> <range value = \"-127 .. 10 | 90 .. 100\"/>  </type>"
            "</typedef>"
            "<leaf name=\"port\">"
            "    <type name=\"my_int_type\"> <range value = \"5 .. 11\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid range restriction - the derived restriction (5 .. 11) is not equally or more limiting.",
            "/TS_ERR2:port", 0);

    /* TEST DEFAULT VALUE */
    schema = MODULE_CREATE_YIN("DF0",
            "<leaf name=\"port\">"
            "    <default value=\"12\" />"
            "    <type name=\"int8\"> <range value = \"min .. 0 | 1 .. 12\"/>  </type>"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "12", 12);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 2, NULL);
    assert_int_equal(range->parts[0].min_64, -128);
    assert_int_equal(range->parts[0].max_64, 0);
    assert_int_equal(range->parts[1].min_64, 1);
    assert_int_equal(range->parts[1].max_64, 12);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "12");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "min .. 0 | 1 .. 12", NULL, NULL, NULL, 0, NULL);

    /* TEST ERROR TD0 */
    schema = MODULE_CREATE_YIN("TD_ERR0",
            "<leaf name=\"port\">"
            "    <default value=\"128\" />"
            "    <type name=\"int8\"> <range value = \"min .. 0 | 1 .. 12\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"128\" is out of type int8 min/max bounds.).",
            "/TD_ERR0:port", 0);

    /* TEST ERROR TD1 */
    schema = MODULE_CREATE_YIN("TD_ERR1",
            "<leaf name=\"port\">"
            "     <default value=\"13\" />"
            "     <type name=\"int8\"> <range value = \"min .. 0 | 1 .. 12\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied range - value \"13\" is out of the allowed range.).",
            "/TD_ERR1:port", 0);

    /* TEST ERROR TD1 */
    schema = MODULE_CREATE_YIN("TD_ERR3",
            "<typedef name= \"my_int_type\">"
            "    <default value=\"10\" />"
            "    <type name=\"int8\"> <range value = \"-127 .. 10 | max\"/> </type>"
            "</typedef>"
            "<leaf name=\"my_leaf\">"
            "     <type name=\"my_int_type\">"
            "     <range value = \"-127 .. -80\"/>  </type>"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Unsatisfied range - value \"10\" is out of the allowed range.).",
            "/TD_ERR3:my_leaf", 0);

    /* TEST DEFAULT VALUE HEXADECIMAL */
    schema = MODULE_CREATE_YIN("DF_HEX0",
            "<leaf name=\"port\">"
            "    <default value=\"+0xf\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "15", 15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "+0xf");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE HEXADECIMAL */
    schema = MODULE_CREATE_YIN("DF_HEX1",
            "<leaf name=\"port\">"
            "    <default value=\"-0xf\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "-15", -15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "-0xf");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YIN("DF_HEX2",
            "<leaf name=\"port\">"
            "    <default value=\"0xff\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0xff\" is out of type int8 min/max bounds.).",
            "/DF_HEX2:port", 0);

    /* TEST DEFAULT VALUE HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YIN("DF_HEX2",
            "<leaf name=\"port\">"
            "    <default value=\"-0x81\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"-0x81\" is out of type int8 min/max bounds.).",
            "/DF_HEX2:port", 0);

    /* TEST DEFAULT VALUE HEXADECIMAL ERROR */
    schema = MODULE_CREATE_YIN("DF_HEX4",
            "<leaf name=\"port\">"
            "    <default value=\"0x80\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0x80\" is out of type int8 min/max bounds.).",
            "/DF_HEX4:port", 0);

    /* TEST DEFAULT VALUE OCTAL */
    schema = MODULE_CREATE_YIN("DF_OCT0",
            "<leaf name=\"port\">"
            "    <default value=\"+017\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "15", 15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "+017");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE OCTAL */
    schema = MODULE_CREATE_YIN("DF_OCT1",
            "<leaf name=\"port\">"
            "    <default value=\"-017\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x205, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, 1);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 0);
    CHECK_LYD_VALUE(*(lysc_leaf->dflt), INT8, "-15", -15);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, "-017");
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "int8", 0, 0, 1, 0, 0, 0);

    /* TEST DEFAULT VALUE OCTAL ERROR */
    schema = MODULE_CREATE_YIN("DF_OCT2",
            "<leaf name=\"port\">"
            "    <default value=\"-0201\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"-0201\" is out of type int8 min/max bounds.).",
            "/DF_OCT2:port", 0);

    /* TEST DEFAULT VALUE OCTAL ERROR */
    schema = MODULE_CREATE_YIN("DF_OCT3",
            "<leaf name=\"port\">"
            "    <default value=\"0200\" />"
            "    <type name=\"int8\" />"
            "</leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Value \"0200\" is out of type int8 min/max bounds.).",
            "/DF_OCT3:port", 0);
}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0",
            "  description \"desc\";\n"
            "leaf port {type int8 {range \"0 .. 50 | 127\";}  default \"20\";}");
    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "\n"
            "  <description>\n"
            "    <text>desc</text>\n"
            "  </description>\n"
            "  <leaf name=\"port\">\n"
            "    <type name=\"int8\">\n"
            "      <range value=\"0 .. 50 | 127\"/>\n"
            "    </type>\n"
            "    <default value=\"20\"/>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT1",
            "\n"
            "  description\n"
            "    \"desc\";\n\n"
            "  leaf port {\n"
            "    type int8 {\n"
            "      range \"0 .. 50 | 127\";\n"
            "    }\n"
            "    default \"20\";\n"
            "  }\n");
    schema_yin = MODULE_CREATE_YIN("PRINT1",
            "<description>"
            "    <text>desc</text>"
            "</description>"
            "<leaf name=\"port\">"
            "    <type name=\"int8\">"
            "        <range value=\"0 .. 50 | 127\"/>"
            "    </type>"
            "<default value=\"20\"/>"
            "</leaf>");

    UTEST_ADD_MODULE(schema_yin, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, schema_yang);
    free(printed);

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT2",
            "  description \"desc\";\n"
            "leaf port {type int8;}");
    schema_yin = MODULE_CREATE_YIN("PRINT2",
            "\n"
            "  <description>\n"
            "    <text>desc</text>\n"
            "  </description>\n"
            "  <leaf name=\"port\">\n"
            "    <type name=\"int8\"/>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT3",
            "\n"
            "  leaf port {\n"
            "    type int8;\n"
            "  }\n");
    schema_yin = MODULE_CREATE_YIN("PRINT3",
            "<leaf name=\"port\">"
            "    <type name=\"int8\"/>"
            "</leaf>");

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
    /* variable for default value test */
    struct lysc_node_container *lysc_root;
    struct lyd_node_inner *lyd_root;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"0 .. 50 | 105\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML("defs", "+50", INT8, "50", 50);
    TEST_SUCCESS_XML("defs", "50", INT8, "50", 50);
    TEST_SUCCESS_XML("defs", "105", INT8, "105", 105);
    TEST_SUCCESS_XML("defs", "0", INT8, "0", 0);
    TEST_SUCCESS_XML("defs", "-0", INT8, "0", 0);
    TEST_ERROR_XML("defs", "-1");
    CHECK_LOG_CTX("Unsatisfied range - value \"-1\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_XML("defs", "51");
    CHECK_LOG_CTX("Unsatisfied range - value \"51\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_XML("defs", "106");
    CHECK_LOG_CTX("Unsatisfied range - value \"106\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_XML("defs", "104");
    CHECK_LOG_CTX("Unsatisfied range - value \"104\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_XML("defs", "60");
    CHECK_LOG_CTX("Unsatisfied range - value \"60\" is out of the allowed range.", "/defs:port", 1);

    schema = MODULE_CREATE_YANG("T0", "leaf port {type int8; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_XML("T0", "-128", INT8, "-128", -128);
    TEST_SUCCESS_XML("T0", "-100", INT8, "-100", -100);
    TEST_SUCCESS_XML("T0", "0", INT8, "0", 0);
    TEST_SUCCESS_XML("T0", "10", INT8, "10", 10);
    TEST_SUCCESS_XML("T0", "50", INT8, "50", 50);
    TEST_SUCCESS_XML("T0", "127", INT8, "127", 127);
    /* leading zeros */
    TEST_SUCCESS_XML("T0", "-015", INT8, "-15", -15);
    TEST_SUCCESS_XML("T0", "015", INT8, "15", 15);
    TEST_ERROR_XML("T0", "-129");
    CHECK_LOG_CTX("Value \"-129\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_XML("T0", "128");
    CHECK_LOG_CTX("Value \"128\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_XML("T0", "256");
    CHECK_LOG_CTX("Value \"256\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_XML("T0", "1024");
    CHECK_LOG_CTX("Value \"1024\" is out of type int8 min/max bounds.", "/T0:port", 1);

    /*
     * default value
     */
    schema = MODULE_CREATE_YANG("T1",
            "container cont {\n"
            "    leaf port {type int8 {range \"0 .. 50 | 105\";} default \"20\";}"
            "}");
    /* check using default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "<cont xmlns=\"urn:tests:" "T1" "\">"  "</cont>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *)tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_root->child, 1, 0, 0, 1, 1,
            INT8, "20", 20);\
    lyd_free_all(tree);

    /* check rewriting default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "<cont xmlns=\"urn:tests:T1\">" "<port> 30 </port>" "</cont>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *)tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_root->child, 0, 0, 0, 1, 1,
            INT8, "30", 30);
    lyd_free_all(tree);

    /*
     * specific error
     */
    schema = MODULE_CREATE_YANG("T2", "leaf port {type int8 {range \"0 .. 50 | 105\" {"
            "        error-app-tag \"range-violation\";"
            "        error-message \"invalid range of value\";"
            "}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_ERROR_XML("T2", "120");
    CHECK_LOG_CTX_APPTAG("invalid range of value", "/T2:port", 1, "range-violation");
}

static void
test_data_json(void **state)
{
    const char *schema;
    /* value for default test */
    struct lysc_node_container *lysc_root;
    struct lyd_node_inner *lyd_root;
    const char *data;
    struct lyd_node *tree;

    /* parsing json data */
    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"0 .. 50 | 105\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_JSON("defs", "50", INT8, "50", 50);
    TEST_SUCCESS_JSON("defs", "50", INT8, "50", 50);
    TEST_SUCCESS_JSON("defs", "105", INT8, "105", 105);
    TEST_SUCCESS_JSON("defs", "0", INT8, "0", 0);
    TEST_SUCCESS_JSON("defs", "-0", INT8, "0", 0);
    TEST_ERROR_JSON("defs", "-1");
    CHECK_LOG_CTX("Unsatisfied range - value \"-1\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_JSON("defs", "51");
    CHECK_LOG_CTX("Unsatisfied range - value \"51\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_JSON("defs", "106");
    CHECK_LOG_CTX("Unsatisfied range - value \"106\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_JSON("defs", "104");
    CHECK_LOG_CTX("Unsatisfied range - value \"104\" is out of the allowed range.", "/defs:port", 1);
    TEST_ERROR_JSON("defs", "60");
    CHECK_LOG_CTX("Unsatisfied range - value \"60\" is out of the allowed range.", "/defs:port", 1);

    schema = MODULE_CREATE_YANG("T0", "leaf port {type int8; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_JSON("T0", "-128", INT8, "-128", -128);
    TEST_SUCCESS_JSON("T0", "-100", INT8, "-100", -100);
    TEST_SUCCESS_JSON("T0", "0", INT8, "0", 0);
    TEST_SUCCESS_JSON("T0", "10", INT8, "10", 10);
    TEST_SUCCESS_JSON("T0", "50", INT8, "50", 50);
    TEST_SUCCESS_JSON("T0", "127", INT8, "127", 127);
    /* leading zeros */
    TEST_ERROR_JSON("T0", "015");
    CHECK_LOG_CTX("Invalid character sequence \"15}\", expected a JSON object-end or next item.", NULL, 1);
    TEST_ERROR_JSON("T0", "-015");
    CHECK_LOG_CTX("Invalid character sequence \"15}\", expected a JSON object-end or next item.", NULL, 1);
    TEST_ERROR_JSON("defs", "+50");
    CHECK_LOG_CTX("Invalid character sequence \"+50}\", expected a JSON value.", NULL, 1);
    TEST_ERROR_JSON("T0", "-129");
    CHECK_LOG_CTX("Value \"-129\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_JSON("T0", "128");
    CHECK_LOG_CTX("Value \"128\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_JSON("T0", "256");
    CHECK_LOG_CTX("Value \"256\" is out of type int8 min/max bounds.", "/T0:port", 1);
    TEST_ERROR_JSON("T0", "1024");
    CHECK_LOG_CTX("Value \"1024\" is out of type int8 min/max bounds.", "/T0:port", 1);

    /*
     * default value
     */
    schema = MODULE_CREATE_YANG("T1",
            "container cont {\n"
            "    leaf port {type int8 {range \"0 .. 50 | 105\";} default \"20\";}"
            "}");
    /* check using default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "{\"T1:cont\":{}}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *)tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_root->child, 1, 0, 0, 1, 1,
            INT8, "20", 20);\
    lyd_free_all(tree);

    /* check rewriting default value */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data = "{\"T1:cont\":{\":port\":30}}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    lysc_root = (void *)tree->schema;
    CHECK_LYSC_NODE(lysc_root->child, NULL, 0, 0x205, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);
    lyd_root = ((struct lyd_node_inner *)tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)lyd_root->child, 0, 0, 0, 1, 1,
            INT8, "30", 30);
    lyd_free_all(tree);

}

static void
test_data_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb", "leaf port {type int8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "port", "-128");
    TEST_SUCCESS_LYB("lyb", "port", "0");
    TEST_SUCCESS_LYB("lyb", "port", "1");
    TEST_SUCCESS_LYB("lyb", "port", "127");
}

static void
test_diff(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"0 .. 50 | 120\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    struct lyd_node *model_1, *model_2;
    struct lyd_node *diff;
    const char *expected_string;
    const char *data_1 = "<port xmlns=\"urn:tests:defs\"> 5 </port>";
    const char *data_2 = "<port xmlns=\"urn:tests:defs\"> 6 </port>";
    const char *diff_expected = "<port xmlns=\"urn:tests:defs\" "
            "xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"5\">"
            "6</port>";

    LYD_TREE_CREATE(data_1, model_1);
    LYD_TREE_CREATE(data_2, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* create data from diff */
    diff_expected = "<port xmlns=\"urn:tests:defs\" "
            "xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"5\">"
            "120</port>";
    LYD_TREE_CREATE(diff_expected, diff);
    data_1 = "<port xmlns=\"urn:tests:defs\"> 5 </port>";
    LYD_TREE_CREATE(data_1, model_1);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    expected_string = "<port xmlns=\"urn:tests:defs\">120</port>";

    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_free_all(model_1);
    lyd_free_all(diff);

    /*
     * check creating data out of range
     */
    diff_expected = "<port xmlns=\"urn:tests:defs\" "
            "xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" "
            "yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"5\">"
            "121</port>";
    CHECK_PARSE_LYD_PARAM(diff_expected, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, model_1);
    CHECK_LOG_CTX("Unsatisfied range - value \"121\" is out of the allowed range.", "/defs:port", 1);

    /*
     * diff from default value
     */
    data_1 = "<cont xmlns=\"urn:tests:T0\"></cont>";
    data_2 = "<cont xmlns=\"urn:tests:T0\"> <port> 6 </port> </cont>";
    diff_expected = "<cont xmlns=\"urn:tests:T0\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"create\"><port>6</port></cont>";

    schema = MODULE_CREATE_YANG("T0",
            "container cont {\n"
            "    leaf port {type int8; default \"20\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    LYD_TREE_CREATE(data_1, model_1);
    LYD_TREE_CREATE(data_2, model_2);
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
    const char *schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"0 .. 50\";}}");
    const char *expected_string;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    struct lyd_node *model_1;
    const char *data_1 = "<port xmlns=\"urn:tests:defs\"> 50 </port>";

    LYD_TREE_CREATE(data_1, model_1);

    /* XML */
    expected_string = "<port xmlns=\"urn:tests:defs\">50</port>";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    /* JSON */
    expected_string = "{\"defs:port\":50}";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    lyd_free_all(model_1);
}

static void
test_plugin_store(void **state)
{
    const char *val_text = NULL;
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value value = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_INT8]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    char *alloc;
    const char *schema;
    struct lysc_type lysc_type_test;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8 {range \"-50 .. 50\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* check proper type */
    assert_string_equal("libyang 2 - integers, version 1", type->id);

    /* check store
     * options = LY_TYPE_STORE_IMPLEMENT | LY_TYPE_STORE_DYNAMIC
     * hint    = LYD_VALHINT_DECNUM, LYD_VALHINT_HEXNUM, LYD_VALHINT_OCTNUM
     */
    val_text = "20";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, INT8, "20", 20);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "-20";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, INT8, "-20", -20);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "0xf";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "15", 15);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "1B";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "27", 27);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "-0xf";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "-15", -15);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "027";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_OCTNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "23", 23);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "-027";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_OCTNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "-23", -23);
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /*
     * minor tests
     * dynamic alocated input text
     */
    val_text = "0xa";
    alloc = (char *)malloc(strlen(val_text) + 1);

    memcpy(alloc, val_text, strlen(val_text) + 1);
    ly_ret = type->store(UTEST_LYCTX, lysc_type, alloc, strlen(val_text),
            LYPLG_TYPE_STORE_DYNAMIC, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    alloc = NULL;
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, INT8, "10", 10);
    type->free(UTEST_LYCTX, &value);

    /* wrong lysc_type of value */
    lysc_type_test = *lysc_type;
    lysc_type_test.basetype = LY_TYPE_UINT8;
    val_text = "20";
    ly_ret = type->store(UTEST_LYCTX, &lysc_type_test, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EINT, ly_ret);
    UTEST_LOG_CTX_CLEAN;

    /*
     * ERROR TESTS
     */
    val_text = "";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, 1,
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "10 b";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "a";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    /* LYPLG_TYPE_STORE_ONLY test */
    val_text = "-60";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            LYPLG_TYPE_STORE_ONLY, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL,
            &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    type->free(UTEST_LYCTX, &value);
    ly_err_free(err);

    UTEST_LOG_CTX_CLEAN;
}

static void
test_plugin_compare(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_INT8]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;
    /* different type */
    const char *diff_type_text = "20";
    struct lyd_value  diff_type_val;
    struct lysc_type *diff_type;
    /* Value which are going to be created to tested */
    const char *val_init[] = {"20", "30", "-30", "0", "-0", "20"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "typedef my_int_type {type int8; }"
            "leaf p1 {type my_int_type;}"
            "leaf p2 {type my_int_type;}"
            "leaf p3 {type my_int_type{range \"0 .. 50\";}}"
            "leaf p4 {type uint8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /*
     * BASIC TEST;
     */
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[0]), &(values[0])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[0]), &(values[5])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[0]), &(values[1])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[2])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[3]), &(values[4])));

    /*
     * SAME TYPE but different node
     */
    diff_type_text = "20";
    diff_type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    ly_ret = diff_type->plugin->store(UTEST_LYCTX, diff_type, diff_type_text, strlen(diff_type_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &diff_type_val, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &diff_type_val, &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[1])));
    type->free(UTEST_LYCTX, &(diff_type_val));

    /*
     * derivated type add some limitations
     */
    diff_type_text = "20";
    diff_type = ((struct lysc_node_leaf *)mod->compiled->data->next->next)->type;
    ly_ret = diff_type->plugin->store(UTEST_LYCTX, diff_type, diff_type_text, strlen(diff_type_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &diff_type_val, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[1])));
    type->free(UTEST_LYCTX, &(diff_type_val));

    /*
     * different type (UINT8)
     */
    diff_type_text = "20";
    diff_type = ((struct lysc_node_leaf *)mod->compiled->data->next->next->next)->type;
    ly_ret = diff_type->plugin->store(UTEST_LYCTX, diff_type, diff_type_text, strlen(diff_type_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_DECNUM, NULL, &diff_type_val, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &diff_type_val, &(values[1])));
    type->free(UTEST_LYCTX, &(diff_type_val));

    /* delete values */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_print(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_INT8]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;
    /* Value which are going to be created to tested */
    const char *val_init[] = {"20", "0x4A", "-f", "0", "-0", "-20"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("defs", "leaf port {type int8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print value */
    ly_bool dynamic = 0;

    assert_string_equal("32", type->print(UTEST_LYCTX, &(values[0]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("74", type->print(UTEST_LYCTX, &(values[1]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("-15", type->print(UTEST_LYCTX, &(values[2]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("0", type->print(UTEST_LYCTX, &(values[3]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("0", type->print(UTEST_LYCTX, &(values[4]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("-32", type->print(UTEST_LYCTX, &(values[5]), LY_VALUE_XML, NULL, &dynamic, NULL));

    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_dup(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_INT8]);
    struct lysc_type *lysc_type[2];
    const char *schema;
    LY_ERR ly_ret;
    /* Value which are going to be tested */
    const char *val_init[] = {"20", "0x4A", "-f", "0", "-0x80", "-20"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type int8;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type[0] = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    schema = MODULE_CREATE_YANG("T1",
            "typedef my_int_type {"
            "    type int8 {range \"-100 .. 100\";} default 20;"
            "}"
            "leaf port {type my_int_type; }");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type[1] = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type[it % 2], val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print duplicate value */
    struct lyd_value dup_value;

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[0]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "32", 0x20);
    assert_ptr_equal(dup_value.realtype, values[0].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[1]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "74", 0x4a);
    assert_ptr_equal(dup_value.realtype, values[1].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[2]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "-15", -0xf);
    assert_ptr_equal(dup_value.realtype, values[2].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[3]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "0", 0x0);
    assert_ptr_equal(dup_value.realtype, values[3].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[4]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "-128", -0x80);
    assert_ptr_equal(dup_value.realtype, values[4].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[5]), &dup_value));
    CHECK_LYD_VALUE(dup_value, INT8, "-32", -0x20);
    assert_ptr_equal(dup_value.realtype, values[5].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    /* error tests */
    assert_int_equal(LY_EINVAL, type->duplicate(NULL, &(values[0]), &dup_value));

    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
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
