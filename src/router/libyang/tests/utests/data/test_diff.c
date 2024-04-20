/**
 * @file test_diff.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for lyd_diff()
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

#define CHECK_PARSE_LYD(INPUT, OUTPUT) \
        CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, OUTPUT)

#define CHECK_LYD_STRING(INPUT, TEXT) \
        CHECK_LYD_STRING_PARAM(INPUT, TEXT, LYD_XML, LYD_PRINT_WITHSIBLINGS)

#define CHECK_PARSE_LYD_DIFF(INPUT_1, INPUT_2, OUT_DIFF) \
        assert_int_equal(LY_SUCCESS, lyd_diff_siblings(INPUT_1, INPUT_2, 0, &OUT_DIFF));\
        assert_non_null(OUT_DIFF)

#define TEST_DIFF_3(XML1, XML2, XML3, DIFF1, DIFF2, MERGE) \
        { \
            struct lyd_node *data1;\
            struct lyd_node *data2;\
            struct lyd_node *data3;\
        /*create*/\
            CHECK_PARSE_LYD(XML1, data1);\
            CHECK_PARSE_LYD(XML2, data2);\
            CHECK_PARSE_LYD(XML3, data3);\
        /* diff1 */ \
            struct lyd_node *diff1;\
            CHECK_PARSE_LYD_DIFF(data1, data2, diff1); \
            CHECK_LYD_STRING(diff1, DIFF1); \
            assert_int_equal(lyd_diff_apply_all(&data1, diff1), LY_SUCCESS); \
            CHECK_LYD(data1, data2); \
        /* diff2 */ \
            struct lyd_node *diff2;\
            CHECK_PARSE_LYD_DIFF(data2, data3, diff2); \
            CHECK_LYD_STRING(diff2, DIFF2); \
            assert_int_equal(lyd_diff_apply_all(&data2, diff2), LY_SUCCESS);\
            CHECK_LYD(data2, data3);\
        /* merge */ \
            assert_int_equal(lyd_diff_merge_all(&diff1, diff2, 0), LY_SUCCESS);\
            CHECK_LYD_STRING(diff1, MERGE); \
        /* cleanup */ \
            lyd_free_all(data1);\
            lyd_free_all(data2);\
            lyd_free_all(data3);\
            lyd_free_all(diff1);\
            lyd_free_all(diff2);\
        }

const char *schema1 =
        "module defaults {\n"
        "    yang-version 1.1;\n"
        "    namespace \"urn:libyang:tests:defaults\";\n"
        "    prefix df;\n"
        ""
        "    feature unhide;\n"
        ""
        "    typedef defint32 {\n"
        "        type int32;\n"
        "        default \"42\";\n"
        "    }\n"
        ""
        "    leaf hiddenleaf {\n"
        "        if-feature \"unhide\";\n"
        "        type int32;\n"
        "        default \"42\";\n"
        "    }\n"
        ""
        "    container df {\n"
        "        leaf foo {\n"
        "            type defint32;\n"
        "        }\n"
        ""
        "        leaf hiddenleaf {\n"
        "            if-feature \"unhide\";\n"
        "            type int32;\n"
        "            default \"42\";\n"
        "        }\n"
        ""
        "        container bar {\n"
        "            presence \"\";\n"
        "            leaf hi {\n"
        "                type int32;\n"
        "                default \"42\";\n"
        "            }\n"
        ""
        "            leaf ho {\n"
        "                type int32;\n"
        "                mandatory true;\n"
        "            }\n"
        "        }\n"
        ""
        "        leaf-list llist {\n"
        "            type defint32;\n"
        "            ordered-by user;\n"
        "        }\n"
        ""
        "        list ul {\n"
        "            key \"l1\";\n"
        "            ordered-by user;\n"
        "            leaf l1 {\n"
        "                type string;\n"
        "            }\n"
        ""
        "            leaf l2 {\n"
        "                type int32;\n"
        "            }\n"
        ""
        "            container cont {\n"
        "                leaf l3 {\n"
        "                    type string;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        ""
        "        leaf-list dllist {\n"
        "            type uint8;\n"
        "            default \"1\";\n"
        "            default \"2\";\n"
        "            default \"3\";\n"
        "        }\n"
        ""
        "        list list {\n"
        "            key \"name\";\n"
        "            leaf name {\n"
        "                type string;\n"
        "            }\n"
        ""
        "            leaf value {\n"
        "                type int32;\n"
        "                default \"42\";\n"
        "            }\n"
        "            list list2 {\n"
        "                key \"name2\";\n"
        "                leaf name2 {\n"
        "                    type string;\n"
        "                }\n"
        "                leaf value2 {\n"
        "                    type int32;\n"
        "                }\n"
        "            }\n"
        "        }\n";
const char *schema2 =
        "        choice select {\n"
        "            default \"a\";\n"
        "            case a {\n"
        "                choice a {\n"
        "                    leaf a1 {\n"
        "                        type int32;\n"
        "                        default \"42\";\n"
        "                    }\n"
        ""
        "                    leaf a2 {\n"
        "                        type int32;\n"
        "                        default \"24\";\n"
        "                    }\n"
        "                }\n"
        "            }\n"
        ""
        "            leaf b {\n"
        "                type string;\n"
        "            }\n"
        ""
        "            container c {\n"
        "                presence \"\";\n"
        "                leaf x {\n"
        "                    type int32;\n"
        "                    default \"42\";\n"
        "                }\n"
        "            }\n"
        "        }\n"
        ""
        "        choice select2 {\n"
        "            default \"s2b\";\n"
        "            leaf s2a {\n"
        "                type int32;\n"
        "                default \"42\";\n"
        "            }\n"
        ""
        "            case s2b {\n"
        "                choice s2b {\n"
        "                    default \"b1\";\n"
        "                    case b1 {\n"
        "                        leaf b1_1 {\n"
        "                            type int32;\n"
        "                            default \"42\";\n"
        "                        }\n"
        ""
        "                        leaf b1_2 {\n"
        "                            type string;\n"
        "                        }\n"
        ""
        "                        leaf b1_status {\n"
        "                            type int32;\n"
        "                            default \"42\";\n"
        "                            config false;\n"
        "                        }\n"
        "                    }\n"
        ""
        "                    leaf b2 {\n"
        "                        type int32;\n"
        "                        default \"42\";\n"
        "                    }\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "        list kl {\n"
        "            config \"false\";\n"
        "            leaf l1 {\n"
        "                type string;\n"
        "            }\n"
        ""
        "            leaf l2 {\n"
        "                type int32;\n"
        "            }\n"
        "        }\n"
        ""
        "        leaf-list kll {\n"
        "            config \"false\";\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        ""
        "    container hidden {\n"
        "        leaf foo {\n"
        "            type int32;\n"
        "            default \"42\";\n"
        "        }\n"
        ""
        "        leaf baz {\n"
        "            type int32;\n"
        "            default \"42\";\n"
        "        }\n"
        ""
        "        leaf papa {\n"
        "            type int32;\n"
        "            default \"42\";\n"
        "            config false;\n"
        "        }\n"
        "    }\n"
        ""
        "    rpc rpc1 {\n"
        "        input {\n"
        "            leaf inleaf1 {\n"
        "                type string;\n"
        "            }\n"
        ""
        "            leaf inleaf2 {\n"
        "                type string;\n"
        "                default \"def1\";\n"
        "            }\n"
        "        }\n"
        ""
        "        output {\n"
        "            leaf outleaf1 {\n"
        "                type string;\n"
        "                default \"def2\";\n"
        "            }\n"
        ""
        "            leaf outleaf2 {\n"
        "                type string;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        ""
        "    notification notif {\n"
        "        leaf ntfleaf1 {\n"
        "            type string;\n"
        "            default \"def3\";\n"
        "        }\n"
        ""
        "        leaf ntfleaf2 {\n"
        "            type string;\n"
        "        }\n"
        "    }\n"
        "}\n";

static int
setup(void **state)
{
    char *schema;

    UTEST_SETUP;

    /* create one schema, longer than 4095 chars */
    schema = malloc(strlen(schema1) + strlen(schema2) + 1);
    strcpy(schema, schema1);
    strcat(schema, schema2);

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    free(schema);

    return 0;
}

static void
test_invalid(void **state)
{
    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    struct lyd_node *model_1;

    CHECK_PARSE_LYD(xml, model_1);

    struct lyd_node *diff = NULL;

    assert_int_equal(lyd_diff_siblings(model_1, lyd_child(model_1), 0, &diff), LY_EINVAL);
    CHECK_LOG_CTX("Invalid arguments - cannot create diff for unrelated data (lyd_diff()).", NULL, 0);

    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, NULL), LY_EINVAL);

    lyd_free_all(model_1);
    lyd_free_all(diff);
}

static void
test_same(void **state)
{
    (void) state;
    const char *xml =
            "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">\n"
            "  <enable-nacm>true</enable-nacm>\n"
            "  <read-default>permit</read-default>\n"
            "  <write-default>deny</write-default>\n"
            "  <exec-default>permit</exec-default>\n"
            "  <enable-external-groups>true</enable-external-groups>\n"
            "</nacm><df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo><b1_1>42</b1_1>\n"
            "</df><hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo><baz>42</baz></hidden>\n";

    struct lyd_node *model_1;
    struct lyd_node *model_2;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-acm", "2018-02-14", NULL));

    CHECK_PARSE_LYD(xml, model_1);
    CHECK_PARSE_LYD(xml, model_2);

    struct lyd_node *diff = NULL;

    assert_int_equal(lyd_diff_siblings(model_1, model_2, 0, &diff), LY_SUCCESS);
    assert_null(diff);
    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    CHECK_LYD(model_1, model_2);

    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);
}

static void
test_empty1(void **state)
{
    (void) state;
    const char *xml_in =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";

    struct lyd_node *model_1 = NULL;
    struct lyd_node *model_2;

    CHECK_PARSE_LYD(xml_in, model_2);

    struct lyd_node *diff;

    CHECK_PARSE_LYD_DIFF(model_1, model_2, diff);
    CHECK_LYD_STRING(diff,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n");
    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    CHECK_LYD(model_1, model_2);

    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);
}

static void
test_empty2(void **state)
{
    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df><hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";

    struct lyd_node *model_1;

    CHECK_PARSE_LYD(xml, model_1);

    struct lyd_node *diff;

    CHECK_PARSE_LYD_DIFF(model_1, NULL, diff);
    CHECK_LYD_STRING(diff,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n");

    assert_int_equal(lyd_diff_apply_all(&model_1, diff), LY_SUCCESS);
    assert_ptr_equal(model_1, NULL);

    lyd_free_all(diff);
    lyd_free_all(model_1);
}

static void
test_empty_nested(void **state)
{
    (void) state;
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";

    struct lyd_node *model_1;

    CHECK_PARSE_LYD(xml, model_1);

    struct lyd_node *diff = NULL;

    assert_int_equal(lyd_diff_siblings(NULL, NULL, 0, &diff), LY_SUCCESS);
    assert_null(diff);

    struct lyd_node *diff1;

    CHECK_PARSE_LYD_DIFF(NULL, lyd_child(model_1), diff1);
    CHECK_LYD_STRING(diff1,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"create\">42</foo>\n"
            "</df>\n");

    struct lyd_node *diff2;

    CHECK_PARSE_LYD_DIFF(lyd_child(model_1), NULL, diff2);
    CHECK_LYD_STRING(diff2,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"delete\">42</foo>\n"
            "</df>\n");

    lyd_free_all(model_1);
    lyd_free_all(diff1);
    lyd_free_all(diff2);
}

static void
test_delete_merge(void **state)
{
    (void) state;
    struct lyd_node *diff1, *diff2;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list>\n"
            "    <name>a</name>\n"
            "    <list2 yang:operation=\"delete\">\n"
            "      <name2>a</name2>\n"
            "    </list2>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>a</name>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>a</name>\n"
            "    <list2 yang:operation=\"delete\">\n"
            "      <name2>a</name2>\n"
            "    </list2>\n"
            "  </list>\n"
            "</df>\n";

    CHECK_PARSE_LYD(xml1, diff1);
    CHECK_PARSE_LYD(xml2, diff2);

    assert_int_equal(lyd_diff_merge_all(&diff1, diff2, 0), LY_SUCCESS);
    CHECK_LYD_STRING(diff1, xml_merge);

    lyd_free_all(diff1);
    lyd_free_all(diff2);
}

static void
test_leaf(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>41</foo>\n"
            "  <b1_1>42</b1_1>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>40</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>40</foo>\n"
            "</hidden>\n";
    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"42\">41</foo>\n"
            "  <b1_1 yang:operation=\"create\">42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"delete\">\n"
            "  <foo>42</foo>\n"
            "  <baz>42</baz>\n"
            "</hidden>\n";

    const char *out_diff_2 = "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"41\">40</foo>\n"
            "  <b1_1 yang:operation=\"delete\">42</b1_1>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"create\">\n"
            "  <foo>40</foo>\n"
            "</hidden>\n";

    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"42\">40</foo>\n"
            "</df>\n"
            "<hidden xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-value=\"42\" yang:orig-default=\"false\">40</foo>\n"
            "  <baz yang:operation=\"delete\">42</baz>\n"
            "</hidden>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_list(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>a</name>\n"
            "    <value>1</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>-2</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <list>\n"
            "    <name>b</name>\n"
            "    <value>-2</value>\n"
            "  </list>\n"
            "  <list>\n"
            "    <name>a</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>a</name>\n"
            "    <value>1</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>b</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\">-2</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"create\">\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>c</name>\n"
            "    <value>3</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"create\">\n"
            "    <name>a</name>\n"
            "    <value>2</value>\n"
            "  </list>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>a</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-value=\"1\" yang:orig-default=\"false\">2</value>\n"
            "  </list>\n"
            "  <list yang:operation=\"none\">\n"
            "    <name>b</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\">-2</value>\n"
            "  </list>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_nested_list(void **state)
{
    struct lyd_node *data1, *data2, *diff;
    const char *xml1, *xml2;

    (void) state;

    xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">"
            "  <list>"
            "    <name>n0</name>"
            "    <value>26</value>"
            "    <list2>"
            "      <name2>n22</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "    <list2>"
            "      <name2>n23</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "  <list>"
            "    <name>n1</name>"
            "    <value>25</value>"
            "    <list2>"
            "      <name2>n22</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "  <list>"
            "    <name>n2</name>"
            "    <value>25</value>"
            "    <list2>"
            "      <name2>n22</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "  <list>"
            "    <name>n3</name>"
            "    <value>25</value>"
            "    <list2>"
            "      <name2>n22</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "  <list>"
            "    <name>n4</name>"
            "    <value>25</value>"
            "    <list2>"
            "      <name2>n22</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "</df>";
    xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">"
            "  <list>"
            "    <name>n0</name>"
            "    <value>30</value>"
            "    <list2>"
            "      <name2>n23</name2>"
            "      <value2>26</value2>"
            "    </list2>"
            "  </list>"
            "</df>";

    CHECK_PARSE_LYD(xml1, data1);
    CHECK_PARSE_LYD(xml2, data2);
    CHECK_PARSE_LYD_DIFF(data1, data2, diff);

    CHECK_LYD_STRING(diff,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <list>\n"
            "    <name>n0</name>\n"
            "    <value yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"26\">30</value>\n"
            "    <list2 yang:operation=\"delete\">\n"
            "      <name2>n22</name2>\n"
            "      <value2>26</value2>\n"
            "    </list2>\n"
            "  </list>\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>n1</name>\n"
            "    <value>25</value>\n"
            "    <list2>\n"
            "      <name2>n22</name2>\n"
            "      <value2>26</value2>\n"
            "    </list2>\n"
            "  </list>\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>n2</name>\n"
            "    <value>25</value>\n"
            "    <list2>\n"
            "      <name2>n22</name2>\n"
            "      <value2>26</value2>\n"
            "    </list2>\n"
            "  </list>\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>n3</name>\n"
            "    <value>25</value>\n"
            "    <list2>\n"
            "      <name2>n22</name2>\n"
            "      <value2>26</value2>\n"
            "    </list2>\n"
            "  </list>\n"
            "  <list yang:operation=\"delete\">\n"
            "    <name>n4</name>\n"
            "    <value>25</value>\n"
            "    <list2>\n"
            "      <name2>n22</name2>\n"
            "      <value2>26</value2>\n"
            "    </list2>\n"
            "  </list>\n"
            "</df>\n");

    lyd_free_all(data1);
    lyd_free_all(data2);
    lyd_free_all(diff);
}

static void
test_userord_llist(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>5</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>5</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>5</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>2</llist>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"\">1</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"\">5</llist>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"1\">4</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"4\">3</llist>\n"
            "  <llist yang:orig-value=\"\" yang:operation=\"delete\">1</llist>\n"
            "  <llist yang:orig-default=\"false\" yang:orig-value=\"2\" yang:value=\"\" yang:operation=\"replace\">5</llist>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_llist2(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "  <llist>4</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>2</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>4</llist>\n"
            "  <llist>1</llist>\n"
            "  <list><name>a</name><value>1</value></list>\n"
            "  <llist>3</llist>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"2\">4</llist>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">4</llist>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\">4</llist>\n"
            "  <llist yang:orig-value=\"1\" yang:operation=\"delete\">2</llist>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_mix(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>2</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>3</llist>\n"
            "  <llist>1</llist>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <llist>1</llist>\n"
            "  <llist>4</llist>\n"
            "  <llist>3</llist>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">3</llist>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\">1</llist>\n"
            "  <llist yang:operation=\"create\" yang:value=\"1\">4</llist>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <llist yang:operation=\"delete\" yang:orig-value=\"1\">2</llist>\n"
            "  <llist yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\" yang:value=\"\">3</llist>\n"
            "  <llist yang:orig-default=\"false\" yang:orig-value=\"3\" yang:value=\"\" yang:operation=\"replace\">1</llist>\n"
            "  <llist yang:value=\"1\" yang:operation=\"create\">4</llist>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_list(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2>11</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>33</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"1\">11</l2>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"delete\" yang:orig-key=\"[l1='a']\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"delete\" yang:orig-key=\"\">\n"
            "    <l1>a</l1>\n"
            "    <l2>11</l2>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"none\">\n"
            "    <l1>c</l1>\n"
            "    <l2 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\">33</l2>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"create\" yang:key=\"[l1='c']\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"delete\">\n"
            "    <l1>a</l1>\n"
            "    <l2 yang:operation=\"delete\">1</l2>\n"
            "  </ul>\n"
            "  <ul yang:orig-key=\"[l1='a']\" yang:operation=\"replace\" yang:key=\"[l1='c']\">\n"
            "    <l1>b</l1>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"none\">\n"
            "    <l1>c</l1>\n"
            "    <l2 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"3\">33</l2>\n"
            "  </ul>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_list2(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>4</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>4</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>4</l2>\n"
            "  </ul>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"create\" yang:key=\"\">\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"create\" yang:key=\"\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"create\" yang:key=\"[l1='a']\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"create\" yang:key=\"\">\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </ul>\n"
            "  <ul yang:key=\"\" yang:operation=\"create\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul yang:key=\"[l1='a']\" yang:operation=\"create\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_userord_list3(void **state)
{
    (void) state;
    const char *xml1 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <cont>\n"
            "      <l3>val1</l3>\n"
            "    </cont>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>4</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml2 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "    <cont>\n"
            "      <l3>val2</l3>\n"
            "    </cont>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>44</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *xml3 =
            "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <ul>\n"
            "    <l1>a</l1>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "    <cont>\n"
            "      <l3>val2</l3>\n"
            "    </cont>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>d</l1>\n"
            "    <l2>44</l2>\n"
            "  </ul>\n"
            "  <ul>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </ul>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"replace\" yang:key=\"\" yang:orig-key=\"[l1='b']\">\n"
            "    <l1>c</l1>\n"
            "    <l2 yang:operation=\"create\">3</l2>\n"
            "    <cont yang:operation=\"none\">\n"
            "      <l3 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"val1\">val2</l3>\n"
            "    </cont>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"replace\" yang:key=\"[l1='a']\" yang:orig-key=\"[l1='b']\">\n"
            "    <l1>d</l1>\n"
            "    <l2 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"4\">44</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"replace\" yang:key=\"\" yang:orig-key=\"[l1='c']\">\n"
            "    <l1>a</l1>\n"
            "    <l2 yang:operation=\"delete\">1</l2>\n"
            "  </ul>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <ul yang:operation=\"replace\" yang:key=\"\" yang:orig-key=\"[l1='b']\">\n"
            "    <l1>c</l1>\n"
            "    <l2 yang:operation=\"create\">3</l2>\n"
            "    <cont yang:operation=\"none\">\n"
            "      <l3 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"val1\">val2</l3>\n"
            "    </cont>\n"
            "  </ul>\n"
            "  <ul yang:operation=\"replace\" yang:key=\"[l1='a']\" yang:orig-key=\"[l1='b']\">\n"
            "    <l1>d</l1>\n"
            "    <l2 yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"4\">44</l2>\n"
            "  </ul>\n"
            "  <ul yang:key=\"\" yang:orig-key=\"[l1='c']\" yang:operation=\"replace\">\n"
            "    <l1>a</l1>\n"
            "    <l2 yang:operation=\"delete\">1</l2>\n"
            "  </ul>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_keyless_list(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kl>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </kl>\n"
            "</df>\n";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kl>\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kl>\n"
            "    <l1>c</l1>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l2>4</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>e</l1>\n"
            "    <l2>5</l2>\n"
            "  </kl>\n"
            "  <kl>\n"
            "    <l1>f</l1>\n"
            "    <l2>6</l2>\n"
            "  </kl>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kl yang:operation=\"delete\" yang:orig-position=\"2\">\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"replace\" yang:position=\"\" yang:orig-position=\"1\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"create\" yang:position=\"2\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kl yang:operation=\"delete\" yang:orig-position=\"\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"delete\" yang:orig-position=\"\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"delete\" yang:orig-position=\"\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"create\" yang:position=\"\">\n"
            "    <l1>c</l1>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"create\" yang:position=\"1\">\n"
            "    <l2>4</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"create\" yang:position=\"2\">\n"
            "    <l1>e</l1>\n"
            "    <l2>5</l2>\n"
            "  </kl>\n"
            "  <kl yang:operation=\"create\" yang:position=\"3\">\n"
            "    <l1>f</l1>\n"
            "    <l2>6</l2>\n"
            "  </kl>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kl yang:operation=\"delete\" yang:orig-position=\"2\">\n"
            "    <l1>c</l1>\n"
            "    <l2>3</l2>\n"
            "  </kl>\n"
            "  <kl yang:orig-position=\"1\" yang:operation=\"delete\">\n"
            "    <l1>b</l1>\n"
            "    <l2>2</l2>\n"
            "  </kl>\n"
            "  <kl yang:orig-position=\"\" yang:operation=\"delete\">\n"
            "    <l1>a</l1>\n"
            "    <l2>1</l2>\n"
            "  </kl>\n"
            "  <kl yang:position=\"\" yang:operation=\"create\">\n"
            "    <l1>c</l1>\n"
            "  </kl>\n"
            "  <kl yang:position=\"1\" yang:operation=\"create\">\n"
            "    <l2>4</l2>\n"
            "  </kl>\n"
            "  <kl yang:position=\"2\" yang:operation=\"create\">\n"
            "    <l1>e</l1>\n"
            "    <l2>5</l2>\n"
            "  </kl>\n"
            "  <kl yang:position=\"3\" yang:operation=\"create\">\n"
            "    <l1>f</l1>\n"
            "    <l2>6</l2>\n"
            "  </kl>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_state_llist(void **state)
{
    (void) state;
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kll>a</kll>\n"
            "  <kll>b</kll>\n"
            "  <kll>c</kll>\n"
            "</df>\n";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kll>b</kll>\n"
            "  <kll>c</kll>\n"
            "  <kll>a</kll>\n"
            "  <kll>a</kll>\n"
            "  <kll>a</kll>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <kll>a</kll>\n"
            "  <kll>d</kll>\n"
            "  <kll>a</kll>\n"
            "</df>\n";

    const char *out_diff_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kll yang:operation=\"replace\" yang:orig-default=\"false\" yang:position=\"\" yang:orig-position=\"1\">b</kll>\n"
            "  <kll yang:operation=\"replace\" yang:orig-default=\"false\" yang:position=\"1\" yang:orig-position=\"2\">c</kll>\n"
            "  <kll yang:operation=\"create\" yang:position=\"3\">a</kll>\n"
            "  <kll yang:operation=\"create\" yang:position=\"4\">a</kll>\n"
            "</df>\n";
    const char *out_diff_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kll yang:operation=\"delete\" yang:orig-position=\"\">b</kll>\n"
            "  <kll yang:operation=\"delete\" yang:orig-position=\"\">c</kll>\n"
            "  <kll yang:operation=\"delete\" yang:orig-position=\"2\">a</kll>\n"
            "  <kll yang:operation=\"create\" yang:position=\"1\">d</kll>\n"
            "</df>\n";
    const char *out_merge =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <kll yang:orig-default=\"false\" yang:orig-position=\"1\" yang:operation=\"delete\">b</kll>\n"
            "  <kll yang:orig-default=\"false\" yang:orig-position=\"2\" yang:operation=\"delete\">c</kll>\n"
            "  <kll yang:operation=\"create\" yang:position=\"4\">a</kll>\n"
            "  <kll yang:position=\"1\" yang:operation=\"create\">d</kll>\n"
            "</df>\n";

    TEST_DIFF_3(xml1, xml2, xml3, out_diff_1, out_diff_2, out_merge);
}

static void
test_wd(void **state)
{
    (void) state;
    const struct lys_module *mod;
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>41</foo>\n"
            "  <dllist>4</dllist>\n"
            "</df>\n";
    const char *xml3 = "<df xmlns=\"urn:libyang:tests:defaults\">\n"
            "  <foo>42</foo>\n"
            "  <dllist>4</dllist>\n"
            "  <dllist>1</dllist>\n"
            "</df>\n";

    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "defaults");
    assert_non_null(mod);

    struct lyd_node *model_1 = NULL;

    assert_int_equal(lyd_validate_module(&model_1, mod, 0, NULL), LY_SUCCESS);
    assert_ptr_not_equal(model_1, NULL);

    struct lyd_node *model_2;
    struct lyd_node *model_3;

    CHECK_PARSE_LYD_PARAM(xml2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    CHECK_PARSE_LYD_PARAM(xml3, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_3);

    /* diff1 */
    struct lyd_node *diff1 = NULL;

    assert_int_equal(lyd_diff_siblings(model_1, model_2, LYD_DIFF_DEFAULTS, &diff1), LY_SUCCESS);
    assert_non_null(diff1);

    const char *diff1_out_1 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"true\" yang:orig-value=\"42\">41</foo>\n"
            "  <dllist yang:operation=\"delete\">1</dllist>\n"
            "  <dllist yang:operation=\"delete\">2</dllist>\n"
            "  <dllist yang:operation=\"delete\">3</dllist>\n"
            "  <dllist yang:operation=\"create\">4</dllist>\n"
            "</df>\n";

    CHECK_LYD_STRING_PARAM(diff1, diff1_out_1, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL);
    assert_int_equal(lyd_diff_apply_all(&model_1, diff1), LY_SUCCESS);
    CHECK_LYD(model_1, model_2);

    /* diff2 */
    struct lyd_node *diff2;

    assert_int_equal(lyd_diff_siblings(model_2, model_3, LYD_DIFF_DEFAULTS, &diff2), LY_SUCCESS);
    assert_non_null(diff2);
    CHECK_LYD_STRING(diff2,
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:operation=\"replace\" yang:orig-default=\"false\" yang:orig-value=\"41\">42</foo>\n"
            "  <dllist yang:operation=\"create\">1</dllist>\n"
            "</df>\n");

    assert_int_equal(lyd_diff_apply_all(&model_2, diff2), LY_SUCCESS);
    CHECK_LYD(model_2, model_3);

    /* merge */
    assert_int_equal(lyd_diff_merge_all(&diff1, diff2, 0), LY_SUCCESS);

    const char *diff1_out_2 =
            "<df xmlns=\"urn:libyang:tests:defaults\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"none\">\n"
            "  <foo yang:orig-default=\"true\" yang:operation=\"none\">42</foo>\n"
            "  <dllist yang:operation=\"none\" yang:orig-default=\"true\">1</dllist>\n"
            "  <dllist yang:operation=\"delete\">2</dllist>\n"
            "  <dllist yang:operation=\"delete\">3</dllist>\n"
            "  <dllist yang:operation=\"create\">4</dllist>\n"
            "</df>\n";

    CHECK_LYD_STRING_PARAM(diff1, diff1_out_2, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_WD_ALL);

    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(model_3);
    lyd_free_all(diff1);
    lyd_free_all(diff2);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_invalid, setup),
        UTEST(test_same, setup),
        UTEST(test_empty1, setup),
        UTEST(test_empty2, setup),
        UTEST(test_empty_nested, setup),
        UTEST(test_delete_merge, setup),
        UTEST(test_leaf, setup),
        UTEST(test_list, setup),
        UTEST(test_nested_list, setup),
        UTEST(test_userord_llist, setup),
        UTEST(test_userord_llist2, setup),
        UTEST(test_userord_mix, setup),
        UTEST(test_userord_list, setup),
        UTEST(test_userord_list2, setup),
        UTEST(test_userord_list3, setup),
        UTEST(test_keyless_list, setup),
        UTEST(test_state_llist, setup),
        UTEST(test_wd, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
