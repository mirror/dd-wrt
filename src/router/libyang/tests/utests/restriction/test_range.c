/**
 * @file test_range.c
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
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, ## __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_SUCCESS_JSON(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "{\"" MOD_NAME ":port\":" DATA "}"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, ## __VA_ARGS__); \
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
    const struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_range *range;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type int8 {"
            "range \"0 .. 50 | 127\"{"
            "description   \"description test\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT8, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "description test", "err-apt-tag", "error message", 0, 2, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int8", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 127", "description test", "err-apt-tag", "error message", 0, NULL);

    /* heredity */
    schema = MODULE_CREATE_YANG("T1", "typedef my_type {type uint16 {"
            "range \"0 .. 100\"{"
            "description   \"percentage\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_UINT16, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "percentage", "err-apt-tag", "error message", 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /* heredity new range */
    schema = MODULE_CREATE_YANG("T2", "typedef my_type {type uint16 {"
            "range \"0 .. 100\"{"
            "description   \"percentage\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}"
            "leaf port {type my_type{range \"0 .. 20\";}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_UINT16, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_type", 0, 0, 1, 1, 0, 0);

    /* change */
    schema = MODULE_CREATE_YANG("T3", "typedef my_type {type uint16 {"
            "range \"0 .. 100\"{"
            "description   \"percentage\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}"
            "leaf port {type my_type{"
            "   range \"0 .. 50\"{"
            "   description   \"description 0-50\";"
            "   error-app-tag \"err-apt-tag 0-50\";"
            "   error-message \"error message 0-50\";}}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_UINT16, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "description 0-50", "err-apt-tag 0-50", "error message 0-50", 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50", "description 0-50", "err-apt-tag 0-50", "error message 0-50", 0, NULL);

}

static void
test_schema_yin(void **state)
{
    const char *schema;
    const struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_range *range;

    schema = MODULE_CREATE_YIN("T0", "<leaf name=\"port\">"
            "<type name=\"int64\">"
            "<range value = \"0 .. 50 | 256\">"
            "   <description>"
            "    <text>desc</text>\n"
            "  </description>\n"
            "<error-app-tag value=\"text &lt; tag\"/>"
            "   <error-message>"
            "       <value>yin error message &lt;</value>\n"
            "   </error-message>\n"
            "</range>"
            "</type>"
            "</leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT64, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "desc", "text < tag", "yin error message <", 0, 2, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "int64", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50 | 256", "desc", "text < tag", "yin error message <", 0, NULL);

    /* heredity */
    schema = MODULE_CREATE_YIN("T1", "<typedef name=\"my_type\">"
            "<type name=\"int16\">"
            "<range value = \"0 .. 50\">"
            "   <description>"
            "    <text>percentage</text>\n"
            "  </description>\n"
            "<error-app-tag value=\"text &lt; tag\"/>"
            "   <error-message>"
            "       <value>yin error message &lt;</value>\n"
            "   </error-message>\n"
            "</range>"
            "</type>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"/> </leaf>");

    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT16, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "percentage", "text < tag", "yin error message <", 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /* heredity new range */
    schema = MODULE_CREATE_YIN("T2", "<typedef name=\"my_type\">"
            "<type name=\"int32\">"
            "<range value = \"0 .. 100\">"
            "   <description>"
            "       <text>percentage</text>\n"
            "   </description>\n"
            "   <error-app-tag value=\"text &lt; tag\"/>"
            "      <error-message>"
            "          <value>yin error message &lt;</value>\n"
            "      </error-message>\n"
            "   </range>"
            "   </type>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "   <range value = \"0 .. 50\"/>"
            "</type></leaf>");

    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT32, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, NULL, NULL, NULL, 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_type", 0, 0, 1, 1, 0, 0);

    /* change */
    schema = MODULE_CREATE_YIN("T3", "<typedef name=\"my_type\">"
            "<type name=\"int32\">"
            "<range value = \"0 .. 100\">"
            "   <description>"
            "       <text>percentage</text>\n"
            "   </description>\n"
            "   <error-app-tag value=\"text &lt; tag\"/>"
            "      <error-message>"
            "          <value>yin error message &lt;</value>\n"
            "      </error-message>\n"
            "   </range>"
            "   </type>"
            "</typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "   <range value = \"0 .. 50\">"
            "   <description>"
            "       <text>percentage 0-50</text>\n"
            "   </description>\n"
            "   <error-app-tag value=\"text tag 0-50\"/>"
            "      <error-message>"
            "          <value>yin error message 0-50</value>\n"
            "      </error-message>\n"
            "   </range>"
            "</type></leaf>");

    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSC_TYPE_NUM((struct lysc_type_num *)lysc_leaf->type, LY_TYPE_INT32, 0, 1);
    range = ((struct lysc_type_num *)lysc_leaf->type)->range;
    CHECK_LYSC_RANGE(range, "percentage 0-50", "text tag 0-50", "yin error message 0-50", 0, 1, NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0x80, 0, 0, "my_type", 0, 0, 1, 1, 0, 0);
    CHECK_LYSP_RESTR(lysp_leaf->type.range, "0 .. 50", "percentage 0-50", "text tag 0-50", "yin error message 0-50", 0, NULL);

}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    const struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0", "leaf port {type int32 {"
            "range \"0 .. 50\"{"
            "description   \"desc < \";"
            "error-app-tag \"err-apt-tag <\";"
            "error-message \"error message <\";}}}");

    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "  <leaf name=\"port\">\n"
            "    <type name=\"int32\">\n"
            "      <range value=\"0 .. 50\">\n"
            "        <error-message>\n"
            "          <value>error message &lt;</value>\n"
            "        </error-message>\n"
            "        <error-app-tag value=\"err-apt-tag &lt;\"/>\n"
            "        <description>\n"
            "          <text>desc &lt; </text>\n"
            "        </description>\n"
            "      </range>\n"
            "    </type>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT1",
            "  leaf port {\n"
            "    type int32 {\n"
            "      range \"0 .. 50\" {\n"
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
            "    <type name=\"int32\">\n"
            "      <range value=\"0 .. 50\">\n"
            "        <error-message>\n"
            "          <value>error message &lt;</value>\n"
            "        </error-message>\n"
            "        <error-app-tag value=\"err-apt-tag &lt;\"/>\n"
            "        <description>\n"
            "          <text>desc &lt; </text>\n"
            "        </description>\n"
            "      </range>\n"
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
    schema = MODULE_CREATE_YANG("TRANGE_0", "leaf port {type int8 {"
            "range \"0 .. 50 | 126\"{"
            "description   \"description test\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* test success */
    TEST_SUCCESS_XML("TRANGE_0", "126", INT8, "126", 126);
    /* test print error */
    TEST_ERROR_XML("TRANGE_0", "-1");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_0:port, line number 1.");
    TEST_ERROR_XML("TRANGE_0", "51");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_0:port, line number 1.");
    TEST_ERROR_XML("TRANGE_0", "127");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_0:port, line number 1.");

    /* xml test */
    schema = MODULE_CREATE_YANG("TRANGE_1", "leaf port {type uint8 {"
            "range \"30 .. 50 | 126\"{"
            "description   \"description test\";"
            "error-app-tag \"err-apt-tag\";"
            "error-message \"error message\";}}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* test success */
    TEST_SUCCESS_XML("TRANGE_1", "126", UINT8, "126", 126);
    /* test print error */
    TEST_ERROR_XML("TRANGE_1", "0");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_1:port, line number 1.");
    TEST_ERROR_XML("TRANGE_1", "51");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_1:port, line number 1.");
    TEST_ERROR_XML("TRANGE_1", "127");
    CHECK_LOG_CTX("error message",
            "Schema location /TRANGE_1:port, line number 1.");

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
