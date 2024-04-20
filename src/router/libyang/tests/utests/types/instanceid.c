/**
 * @file instanceid.c
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
#include "path.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML2(XML1, MOD_NAME, NAMESPACES, NODE_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = XML1 "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 1, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML2(XML1, MOD_NAME, NAMESPACES, NODE_NAME, DATA, RET) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" " NAMESPACES ">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, RET, tree); \
        assert_null(tree); \
    }

#define LYB_CHECK_START \
    struct lyd_node *tree_1; \
    struct lyd_node *tree_2; \
    char *xml_out, *data;

#define LYB_CHECK_END \
    { \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME1, DATA1, NODE_NAME2, DATA2) \
    LYB_CHECK_START \
    data = "<" NODE_NAME1 " xmlns=\"urn:tests:" MOD_NAME "\">" DATA1 "</" NODE_NAME1 ">" \
    "<xdf:" NODE_NAME2 " xmlns:xdf=\"urn:tests:" MOD_NAME "\">/xdf:" DATA2 "</xdf:" NODE_NAME2 ">"; \
    LYB_CHECK_END \

#define TEST_SUCCESS_LYB2(MOD_NAME, NODE_NAME, DATA) \
    { \
        LYB_CHECK_START \
        data = "<" NODE_NAME " xmlns:aa=\"urn:tests:lyb2\" xmlns=\"urn:tests:" MOD_NAME "\">/aa:" DATA "</" NODE_NAME ">"; \
        LYB_CHECK_END \
    }

static void
test_data_xml(void **state)
{
    const char *schema, *schema2;
    const enum ly_path_pred_type val1[] = {0, 0};
    const enum ly_path_pred_type val2[] = {LY_PATH_PREDTYPE_LIST, 0};
    const enum ly_path_pred_type val3[] = {LY_PATH_PREDTYPE_LEAFLIST};
    const enum ly_path_pred_type val4[] = {LY_PATH_PREDTYPE_LIST, 0};
    const enum ly_path_pred_type val5[] = {LY_PATH_PREDTYPE_LIST, 0};
    const enum ly_path_pred_type val6[] = {LY_PATH_PREDTYPE_LIST, 0};

    /* xml test */
    schema = MODULE_CREATE_YANG("mod", "container cont {leaf l2 {type empty;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    schema2 = MODULE_CREATE_YANG("defs", "identity ident; identity ident-der1 {base ident;} identity ident-der2 {base ident;}"
            "leaf l1 {type instance-identifier {require-instance true;}}"
            "leaf l2 {type instance-identifier {require-instance false;}}"
            "container cont {leaf l {type empty;}}"
            "list list {key \"id\"; leaf id {type string;} leaf value {type string;}}"
            "leaf-list llist {type uint32;}"
            "list list-inst {key \"id\"; leaf id {type instance-identifier;} leaf value {type string;}}"
            "list list-ident {key \"id\"; leaf id {type identityref {base ident;}} leaf value {type string;}}"
            "list list2 {key \"id id2\"; leaf id {type string;} leaf id2 {type string;}}"
            "list list-keyless {config false; leaf value {type string;}}");
    UTEST_ADD_MODULE(schema2, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML2("<cont xmlns=\"urn:tests:defs\"><l/></cont>", "defs", "xmlns:xdf=\"urn:tests:defs\"", "l1",
            "/xdf:cont/xdf:l", INST, "/defs:cont/l", val1);

    TEST_SUCCESS_XML2("<list xmlns=\"urn:tests:defs\"><id>a</id></list><list xmlns=\"urn:tests:defs\"><id>b</id></list>",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/xdf:list[xdf:id='b']/xdf:id", INST,
            "/defs:list[id='b']/id", val2);

    TEST_SUCCESS_XML2("<llist xmlns=\"urn:tests:defs\">1</llist><llist xmlns=\"urn:tests:defs\">2</llist>",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/xdf:llist[.='1']", INST, "/defs:llist[.='1']", val3);

    TEST_SUCCESS_XML2("<list-inst xmlns=\"urn:tests:defs\"><id xmlns:b=\"urn:tests:defs\">/b:llist[.='1']</id>"
            "<value>x</value></list-inst>"
            "<list-inst xmlns=\"urn:tests:defs\"><id xmlns:b=\"urn:tests:defs\">/b:llist[.='2']</id>"
            "<value>y</value></list-inst>"
            "<llist xmlns=\"urn:tests:defs\">1</llist><llist xmlns=\"urn:tests:defs\">2</llist>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "a:l1", "/a:list-inst[a:id=\"/a:llist[.='1']\"]/a:value",
            INST, "/defs:list-inst[id=\"/defs:llist[.='1']\"]/value", val4);

    TEST_SUCCESS_XML2("<list-ident xmlns=\"urn:tests:defs\"><id xmlns:b=\"urn:tests:defs\">b:ident-der1</id>"
            "<value>x</value></list-ident>"
            "<list-ident xmlns=\"urn:tests:defs\"><id xmlns:b=\"urn:tests:defs\">b:ident-der2</id>"
            "<value>y</value></list-ident>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "a:l1", "/a:list-ident[a:id='a:ident-der1']/a:value",
            INST, "/defs:list-ident[id='defs:ident-der1']/value", val5);

    TEST_SUCCESS_XML2("<list2 xmlns=\"urn:tests:defs\"><id>defs:xxx</id><id2>x</id2></list2>"
            "<list2 xmlns=\"urn:tests:defs\"><id>a:xxx</id><id2>y</id2></list2>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "a:l1", "/a:list2[a:id='a:xxx'][a:id2='y']/a:id2",
            INST, "/defs:list2[id='a:xxx'][id2='y']/id2", val6);

    /* syntax/semantic errors */
    TEST_ERROR_XML2("<list xmlns=\"urn:tests:defs\"><id>a</id></list>"
            "<list xmlns=\"urn:tests:defs\"><id>b</id><value>x</value></list>",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/xdf:list[2]/xdf:value", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list[2]/xdf:value\" value - semantic error: "
            "Positional predicate defined for configuration list \"list\" in path.", "/defs:l1", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/t:cont/t:1l", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:1l\" value - syntax error: Invalid character 't'[9] of expression '/t:cont/t:1l'.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/t:cont:t:1l", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont:t:1l\" value - syntax error: Invalid character ':'[8] of expression '/t:cont:t:1l'.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l1", "/xdf:cont/xdf:invalid/xdf:path", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:cont/xdf:invalid/xdf:path\" value - semantic error: Not found node \"invalid\" in path.",
            "/defs:l1", 1);

    /* non-existing instances, instance-identifier is here in JSON format because it is already in internal
     * representation without canonical prefixes */
    TEST_ERROR_XML2("<cont xmlns=\"urn:tests:mod\"/>",
            "defs", "xmlns:m=\"urn:tests:mod\"", "l1", "/m:cont/m:l2", LY_ENOTFOUND);
    CHECK_LOG_CTX_APPTAG("Invalid instance-identifier \"/mod:cont/l2\" value - required instance not found.",
            "/defs:l1", 0, "instance-required");

    TEST_ERROR_XML2("<llist xmlns=\"urn:tests:defs\">1</llist>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "l1", "/a:llist[.='2']", LY_ENOTFOUND);
    CHECK_LOG_CTX_APPTAG("Invalid instance-identifier \"/defs:llist[.='2']\" value - required instance not found.",
            "/defs:l1", 0, "instance-required");

    TEST_ERROR_XML2("<list2 xmlns=\"urn:tests:defs\"><id>a</id><id2>a</id2></list2>"
            "<list2 xmlns=\"urn:tests:defs\"><id>c</id><id2>b</id2></list2>"
            "<llist xmlns=\"urn:tests:defs\">a</llist>"
            "<llist xmlns=\"urn:tests:defs\">b</llist>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "l1", "/a:list2[a:id='a'][a:id2='a']/a:id", LY_ENOTFOUND);
    CHECK_LOG_CTX_APPTAG("Invalid instance-identifier \"/defs:list2[id='a'][id2='a']/id\" value - required instance not found.",
            "/defs:l1", 0, "instance-required");

    TEST_ERROR_XML2("<list2 xmlns=\"urn:tests:defs\"><id>a</id><id2>a</id2></list2>"
            "<list2 xmlns=\"urn:tests:defs\"><id>c</id><id2>b</id2></list2>"
            "<llist xmlns=\"urn:tests:defs\">1</llist>"
            "<llist xmlns=\"urn:tests:defs\">2</llist>",
            "defs", "xmlns:a=\"urn:tests:defs\"", "l1", "/a:llist[.='3']", LY_ENOTFOUND);
    CHECK_LOG_CTX_APPTAG("Invalid instance-identifier \"/defs:llist[.='3']\" value - required instance not found.",
            "/defs:l1", 0, "instance-required");

    TEST_ERROR_XML2("",
            "defs", "xmlns:a=\"urn:tests:defs\"", "l1", "/a:list-keyless[3]", LY_ENOTFOUND);
    CHECK_LOG_CTX_APPTAG("Invalid instance-identifier \"/defs:list-keyless[3]\" value - required instance not found.",
            "/defs:l1", 0, "instance-required");

    /* more errors */
    TEST_ERROR_XML2("<llist xmlns=\"urn:tests:defs\">x</llist>",
            "defs", "xmlns:t=\"urn:tests:defs\"", "t:l1", "/t:llist[1", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[1\" value - syntax error: Unexpected XPath expression end.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("<cont xmlns=\"urn:tests:mod\"/>",
            "defs", "xmlns:m=\"urn:tests:mod\"", "l1", "/m:cont[1]", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/m:cont[1]\" value - semantic error: Positional predicate defined for container \"cont\" in path.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("<cont xmlns=\"urn:tests:mod\"/>",
            "defs", "xmlns:m=\"urn:tests:mod\"", "l1", "[1]", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"[1]\" value - syntax error: Unexpected XPath token \"[\" (\"[1]\"), expected \"Operator(Path)\".",
            "/defs:l1", 1);

    TEST_ERROR_XML2("<cont xmlns=\"urn:tests:mod\"><l2/></cont>",
            "defs", "xmlns:m=\"urn:tests:mod\"", "l1", "/m:cont/m:l2[l2='1']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/m:cont/m:l2[l2='1']\" value - syntax error: Prefix missing for \"l2\" in path.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("<cont xmlns=\"urn:tests:mod\"><l2/></cont>",
            "defs", "xmlns:m=\"urn:tests:mod\"", "l1", "/m:cont/m:l2[m:l2='1']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/m:cont/m:l2[m:l2='1']\" value - semantic error: List predicate defined for leaf \"l2\" in path.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("<llist xmlns=\"urn:tests:defs\">1</llist><llist xmlns=\"urn:tests:defs\">2</llist>",
            "defs", "xmlns:t=\"urn:tests:defs\"", "t:l1", "/t:llist[4]", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[4]\" value - semantic error: Positional predicate defined for configuration leaf-list \"llist\" in path.",
            "/defs:l1", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/t:llist[6]", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[6]\" value - semantic error: No module connected with the prefix \"t\" found (prefix format XML prefixes).",
            "/defs:l2", 1);

    TEST_ERROR_XML2("<list xmlns=\"urn:tests:defs\"><id>1</id><value>x</value></list>",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/xdf:list[xdf:value='x']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list[xdf:value='x']\" value - semantic error: Key expected instead of leaf \"value\" in path.",
            "/defs:l2", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/xdf:list[.='x']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list[.='x']\" value - semantic error: Leaf-list predicate defined for list \"list\" in path.",
            "/defs:l2", 1);

    TEST_ERROR_XML2("<llist xmlns=\"urn:tests:defs\">1</llist>",
            "defs", "xmlns:t=\"urn:tests:defs\"", "t:l1", "/t:llist[.='x']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[.='x']\" value - semantic error: Invalid type uint32 value \"x\".",
            "/defs:l1", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/t:llist[1][2]", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[1][2]\" value - syntax error: Unparsed characters \"[2]\" left at the end of path.",
            "/defs:l2", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/t:llist[.='a'][.='b']", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:llist[.='a'][.='b']\" value - syntax error: Unparsed characters \"[.='b']\" left at the end of path.",
            "/defs:l2", 1);

    TEST_ERROR_XML2("<list xmlns=\"urn:tests:defs\"><id>1</id><value>x</value></list>",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/xdf:list[xdf:id='1'][xdf:id='2']/xdf:value", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list[xdf:id='1'][xdf:id='2']/xdf:value\" value - syntax error: Duplicate predicate key \"id\" in path.",
            "/defs:l2", 1);

    TEST_ERROR_XML2("",
            "defs", "xmlns:xdf=\"urn:tests:defs\"", "xdf:l2", "/xdf:list2[xdf:id='1']/xdf:value", LY_EVALID);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list2[xdf:id='1']/xdf:value\" value - semantic error: Predicate missing for a key of list \"list2\" in path.",
            "/defs:l2", 1);
}

static void
test_plugin_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "leaf-list leaflisttarget {type string;}"
            "leaf inst {type instance-identifier {require-instance true;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "leaflisttarget", "1", "inst", "leaflisttarget[.='1']");

    /* ietf-netconf-acm node-instance-identifier type */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    schema = MODULE_CREATE_YANG("lyb2",
            "import ietf-netconf-acm {prefix acm;}"
            "leaf-list ll {type string;}"
            "leaf nii {type acm:node-instance-identifier;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB2("lyb2", "nii", "ll[. = 'some_string']");
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
