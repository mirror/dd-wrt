/**
 * @file identityref.c
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

#define TEST_ERROR_XML(MOD_NAME, NAMESPACES, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
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

static void
test_data_xml(void **state)
{
    const char *schema, *schema2;
    struct lyd_node *tree;
    const char *data;

    /* xml test */
    schema = "module ident-base {"
            "  yang-version 1.1;"
            "  namespace \"urn:tests:ident-base\";"
            "  prefix ib;"
            "  identity ident-base;"
            "  identity ident-imp {base ident-base;}"
            "}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    schema2 = "module defs {"
            "  yang-version 1.1;"
            "  namespace \"urn:tests:defs\";"
            "  prefix d;"
            "  import ident-base {prefix ib;}"
            "  identity ident1 {base ib:ident-base;}"
            "  leaf l1 {type identityref {base ib:ident-base;}}"
            "}";
    UTEST_ADD_MODULE(schema2, LYS_IN_YANG, NULL, NULL);

    /* local ident, XML/JSON print */
    data = "<l1 xmlns=\"urn:tests:defs\">ident1</l1>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, IDENT, "defs:ident1", "ident1");
    CHECK_LYD_STRING_PARAM(tree, data, LYD_XML, LYD_PRINT_SHRINK);
    CHECK_LYD_STRING_PARAM(tree, "{\"defs:l1\":\"ident1\"}", LYD_JSON, LYD_PRINT_SHRINK);
    lyd_free_all(tree);

    /* foreign ident, XML/JSON print */
    data = "<l1 xmlns=\"urn:tests:defs\" xmlns:ib=\"urn:tests:ident-base\">ib:ident-imp</l1>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, IDENT, "ident-base:ident-imp", "ident-imp");
    CHECK_LYD_STRING_PARAM(tree, data, LYD_XML, LYD_PRINT_SHRINK);
    CHECK_LYD_STRING_PARAM(tree, "{\"defs:l1\":\"ident-base:ident-imp\"}", LYD_JSON, LYD_PRINT_SHRINK);
    lyd_free_all(tree);

    /* invalid value */
    TEST_ERROR_XML("defs", "", "l1", "fast-ethernet");
    CHECK_LOG_CTX("Invalid identityref \"fast-ethernet\" value - identity not found in module \"defs\".", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "xmlns:x=\"urn:tests:defs\"", "l1", "x:slow-ethernet");
    CHECK_LOG_CTX("Invalid identityref \"x:slow-ethernet\" value - identity not found in module \"defs\".", "/defs:l1", 1);

    TEST_ERROR_XML("defs", "xmlns:x=\"urn:tests:ident-base\"", "l1", "x:ident-base");
    CHECK_LOG_CTX("Invalid identityref \"x:ident-base\" value - identity not derived from the base \"ident-base:ident-base\".",
            "/defs:l1", 1);

    TEST_ERROR_XML("defs", "xmlns:x=\"urn:tests:unknown\"", "l1", "x:ident-base");
    CHECK_LOG_CTX("Invalid identityref \"x:ident-base\" value - unable to map prefix to YANG schema.", "/defs:l1", 1);
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "identity idbase;"
            "identity ident {base idbase;}"
            "leaf lf {type identityref {base idbase;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "lf", "ident");
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
