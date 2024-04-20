/**
 * @file decimal64.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief test for built-in enumeration type
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

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML(MOD_NAME, NODE_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
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

#define TEST_SUCCESS_PARSE_STORE_ONLY_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_STORE_ONLY, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        lyd_free_all(tree); \
    }

static void
test_data_xml(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "leaf l1 {type decimal64 {fraction-digits 1; range 1.5..10;}}"
            "leaf l2 {type decimal64 {fraction-digits 18;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML("defs", "l1", "\n +8 \t\n  ", DEC64, "8.0", 80);
    TEST_SUCCESS_XML("defs", "l1", "8.00", DEC64, "8.0", 80);

    TEST_SUCCESS_XML("defs", "l2", "-9.223372036854775808", DEC64, "-9.223372036854775808",
            INT64_C(-9223372036854775807) - INT64_C(1));
    TEST_SUCCESS_XML("defs", "l2", "9.223372036854775807", DEC64, "9.223372036854775807", INT64_C(9223372036854775807));

    TEST_ERROR_XML("defs", "l1", "\n 15 \t\n  ");
    CHECK_LOG_CTX("Unsatisfied range - value \"15.0\" is out of the allowed range.", "/defs:l1", 3);

    TEST_ERROR_XML("defs", "l1", "\n 0 \t\n  ");
    CHECK_LOG_CTX("Unsatisfied range - value \"0.0\" is out of the allowed range.", "/defs:l1", 3);

    TEST_ERROR_XML("defs", "l1", "xxx");
    CHECK_LOG_CTX("Invalid 1. character of decimal64 value \"xxx\".", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "l1", "");
    CHECK_LOG_CTX("Invalid empty decimal64 value.", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "l1", "8.5  xxx");
    CHECK_LOG_CTX("Invalid 6. character of decimal64 value \"8.5  xxx\".", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "l1", "8.55  xxx");
    CHECK_LOG_CTX("Value \"8.55\" of decimal64 type exceeds defined number (1) of fraction digits.", "/defs:l1", 1);

    /* LYPLG_TYPE_STORE_ONLY test */
    TEST_SUCCESS_PARSE_STORE_ONLY_XML("defs", "l1", "\n 15 \t\n  ");
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "leaf dec64 {type decimal64 {fraction-digits 1; range 1.5..10;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "dec64", "8.00");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_plugin_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
