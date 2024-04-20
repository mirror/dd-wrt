/**
 * @file test_pattern.c
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
    struct lysc_pattern *pattern;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type string {"
            "pattern \"[A-Za-z]*\"{"
            "description   \"pattern description\";"
            "error-app-tag \"pattern err-apt-tag\";"
            "error-message \"pattern error message\";}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Za-z]*", "pattern description",
            "pattern err-apt-tag", "pattern error message", 0, NULL);

    /* heredity */
    schema = MODULE_CREATE_YANG("T1", "typedef my_type {type string {"
            "pattern \"[A-Za-z]*\"{"
            "description   \"pattern description\";"
            "error-app-tag \"pattern err-apt-tag\";"
            "error-message \"pattern error message\";}}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /* heredity new pattern */
    schema = MODULE_CREATE_YANG("T2", "typedef my_type {type string {"
            "pattern \"[A-Za-z]*\"{"
            "description   \"pattern description\";"
            "error-app-tag \"pattern err-apt-tag\";"
            "error-message \"pattern error message\";}}}"
            "leaf port {type my_type{pattern \"[A-Z]*\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[A-Z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Z]*", NULL, NULL, NULL, 0, NULL);

    /* heredity new pattern */
    schema = MODULE_CREATE_YANG("T3", "typedef my_type {type string {"
            "pattern \"[A-Za-z]*\"{"
            "   description   \"pattern 0 description\";"
            "   error-app-tag \"pattern 0 err-apt-tag\";"
            "   error-message \"pattern 0 error message\";}}}"
            "leaf port {type my_type{pattern \"[A-Z]*\"{"
            "   description   \"pattern 1 description\";"
            "   error-app-tag \"pattern 1 err-apt-tag\";"
            "   error-message \"pattern 1 error message\";"
            "}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern 0 description", "pattern 0 err-apt-tag",
            "pattern 0 error message", "[A-Za-z]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, "pattern 1 description", "pattern 1 err-apt-tag",
            "pattern 1 error message", "[A-Z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Z]*", "pattern 1 description",
            "pattern 1 err-apt-tag", "pattern 1 error message", 0, NULL);
}

static void
test_schema_yin(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_pattern *pattern;

    schema = MODULE_CREATE_YIN("T0", "<leaf name=\"port\"> <type name=\"string\">"
            "<pattern value=\"[A-Za-z]*\">"
            "   <description><text>pattern description</text></description>"
            "   <error-app-tag value=\"pattern err-apt-tag\"/>"
            "   <error-message> <value>pattern error message</value></error-message>"
            "</pattern></type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "string", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Za-z]*", "pattern description",
            "pattern err-apt-tag", "pattern error message", 0, NULL);

    /* heredity */
    schema = MODULE_CREATE_YIN("T1", "<typedef name=\"my_type\"> <type name=\"string\">"
            "<pattern value=\"[A-Za-z]*\">"
            "   <description><text>pattern description</text></description>"
            "   <error-app-tag value=\"pattern err-apt-tag\"/>"
            "   <error-message><value>pattern error message</value></error-message>"
            "</pattern></type></typedef>"
            "<leaf name=\"port\"><type name=\"my_type\"/></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 1);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /* heredity new pattern */
    schema = MODULE_CREATE_YIN("T2", "<typedef name=\"my_type\"> <type name=\"string\">"
            "<pattern value=\"[A-Za-z]*\">"
            "   <description><text>pattern description</text></description>"
            "   <error-app-tag value=\"pattern err-apt-tag\"/>"
            "   <error-message><value>pattern error message</value></error-message>"
            "</pattern></type></typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"><pattern value=\"[A-Z]*\"/>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern description", "pattern err-apt-tag",
            "pattern error message", "[A-Za-z]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, NULL, NULL, NULL, "[A-Z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Z]*", NULL, NULL, NULL, 0, NULL);

    /* heredity new pattern */
    schema = MODULE_CREATE_YIN("T3", "<typedef name=\"my_type\"> <type name=\"string\">"
            "<pattern value=\"[A-Za-z]*\">"
            "   <description> <text>pattern 0 description</text></description>"
            "   <error-app-tag value=\"pattern 0 err-apt-tag\"/>"
            "   <error-message> <value>pattern 0 error message</value></error-message>"
            "</pattern></type></typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "<pattern value=\"[A-Z]*\">"
            "   <description><text>pattern 1 description</text></description>"
            "   <error-app-tag value=\"pattern 1 err-apt-tag\"/>"
            "   <error-message><value>pattern 1 error message</value></error-message>"
            "</pattern></type></leaf>");

    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *) mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_STR((struct lysc_type_str *)lysc_leaf->type, 0, 0, 2);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[0];
    CHECK_LYSC_PATTERN(pattern, "pattern 0 description", "pattern 0 err-apt-tag",
            "pattern 0 error message", "[A-Za-z]*", 0, 0, NULL);
    pattern = ((struct lysc_type_str *)lysc_leaf->type)->patterns[1];
    CHECK_LYSC_PATTERN(pattern, "pattern 1 description", "pattern 1 err-apt-tag",
            "pattern 1 error message", "[A-Z]*", 0, 0, NULL);
    lysp_leaf = (void *) mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x40, 0, 0, "my_type", 0, 1, 1, 0, 0, 0);
    CHECK_LYSP_RESTR(&(lysp_leaf->type.patterns[0]), "\x6" "[A-Z]*", "pattern 1 description",
            "pattern 1 err-apt-tag", "pattern 1 error message", 0, NULL);
}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0", "leaf port {type string {"
            "pattern \"[A-Z]*\"{"
            "description   \"desc < \";"
            "error-app-tag \"err-apt-tag <\";"
            "error-message \"error message <\";}}}");

    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "  <leaf name=\"port\">\n"
            "    <type name=\"string\">\n"
            "      <pattern value=\"[A-Z]*\">\n"
            "        <error-message>\n"
            "          <value>error message &lt;</value>\n"
            "        </error-message>\n"
            "        <error-app-tag value=\"err-apt-tag &lt;\"/>\n"
            "        <description>\n"
            "          <text>desc &lt; </text>\n"
            "        </description>\n"
            "      </pattern>\n"
            "    </type>\n"
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
            "      pattern \"[A-Z]*\" {\n"
            "        error-message\n"
            "          \"error message <\";\n"
            "        error-app-tag \"err-apt-tag <\";\n"
            "        description\n"
            "          \"desc < \";\n"
            "      }\n"
            "    }\n"
            "  }\n");

    schema_yin = MODULE_CREATE_YIN("PRINT1",
            "  <leaf name=\"port\">\n"
            "    <type name=\"string\">\n"
            "      <pattern value=\"[A-Z]*\">\n"
            "        <error-message>\n"
            "          <value>error message &lt;</value>\n"
            "        </error-message>\n"
            "        <error-app-tag value=\"err-apt-tag &lt;\"/>\n"
            "        <description>\n"
            "          <text>desc &lt; </text>\n"
            "        </description>\n"
            "      </pattern>\n"
            "    </type>\n"
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

    /* xml test */
    schema = MODULE_CREATE_YANG("TPATTERN_0", "typedef my_type {type string {"
            "pattern \"[A-Za-z]*\"{"
            "   description   \"pattern 0 description\";"
            "   error-app-tag \"pattern 0 err-apt-tag\";"
            "   error-message \"pattern 0 error message\";}}}"
            "leaf port {type my_type{pattern \"[A-Z]*\"{"
            "   description   \"pattern 1 description\";"
            "   error-app-tag \"pattern 1 err-apt-tag\";"
            "   error-message \"pattern 1 error message\";"
            "}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* test success */
    TEST_SUCCESS_XML("TPATTERN_0", "AHOJ", STRING, "AHOJ");
    /* test print error */
    TEST_ERROR_XML("TPATTERN_0", "T128");
    CHECK_LOG_CTX("pattern 0 error message", "/TPATTERN_0:port", 1);
    TEST_ERROR_XML("TPATTERN_0", "ahoj");
    CHECK_LOG_CTX("pattern 1 error message", "/TPATTERN_0:port", 1);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema_yang),
        UTEST(test_schema_yin),
        UTEST(test_schema_print),
        UTEST(test_data_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
