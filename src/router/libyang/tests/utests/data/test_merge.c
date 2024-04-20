/**
 * @file test_merge.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for complex data merges.
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

#include "libyang.h"

#define LYD_TREE_CREATE(INPUT, MODEL) \
                CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, MODEL)

#define CONTEXT_CREATE \
                CONTEXT_CREATE_PATH(NULL)

#define LYD_TREE_CHECK_CHAR(MODEL, TEXT, PARAMS) \
                CHECK_LYD_STRING_PARAM(MODEL, TEXT, LYD_XML, LYD_PRINT_WITHSIBLINGS | PARAMS)

static void
test_batch(void **state)
{
    const char *start =
            "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module>\n"
            "    <name>yang</name>\n"
            "    <revision>2016-02-11</revision>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "</modules-state>\n";
    const char *data[] = {
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-yang-library</name>\n"
        "    <revision>2016-02-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-acm</name>\n"
        "    <revision>2012-02-22</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-monitoring</name>\n"
        "    <revision>2010-10-04</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-with-defaults</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>yang</name>\n"
        "    <revision>2016-02-11</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-yang-library</name>\n"
        "    <revision>2016-02-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-acm</name>\n"
        "    <revision>2012-02-22</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
        "    <feature>writable-running</feature>\n"
        "    <feature>candidate</feature>\n"
        "    <feature>rollback-on-error</feature>\n"
        "    <feature>validate</feature>\n"
        "    <feature>startup</feature>\n"
        "    <feature>xpath</feature>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-monitoring</name>\n"
        "    <revision>2010-10-04</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n",
        "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
        "  <module>\n"
        "    <name>ietf-netconf-with-defaults</name>\n"
        "    <revision>2011-06-01</revision>\n"
        "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>\n"
        "    <conformance-type>implement</conformance-type>\n"
        "  </module>\n"
        "</modules-state>\n"
    };
    const char *output_template =
            "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module>\n"
            "    <name>yang</name>\n"
            "    <revision>2016-02-11</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-yang-library</name>\n"
            "    <revision>2016-02-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-acm</name>\n"
            "    <revision>2012-02-22</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "    <feature>writable-running</feature>\n"
            "    <feature>candidate</feature>\n"
            "    <feature>rollback-on-error</feature>\n"
            "    <feature>validate</feature>\n"
            "    <feature>startup</feature>\n"
            "    <feature>xpath</feature>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-monitoring</name>\n"
            "    <revision>2010-10-04</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-with-defaults</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "</modules-state>\n";

    struct lyd_node *target;

    CHECK_PARSE_LYD_PARAM(start, LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, target);

    for (int32_t i = 0; i < 11; ++i) {
        struct lyd_node *source;

        CHECK_PARSE_LYD_PARAM(data[i], LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, source);
        assert_int_equal(LY_SUCCESS, lyd_merge_siblings(&target, source, LYD_MERGE_DESTRUCT));
    }

    LYD_TREE_CHECK_CHAR(target, output_template, 0);

    lyd_free_all(target);
}

static void
test_leaf(void **state)
{
    const char *sch = "module x {"
            "  namespace urn:x;"
            "  prefix x;"
            "    container A {"
            "      leaf f1 {type string;}"
            "      container B {"
            "        leaf f2 {type string;}"
            "      }"
            "    }"
            "  }";
    const char *trg = "<A xmlns=\"urn:x\"> <f1>block</f1> </A>";
    const char *src = "<A xmlns=\"urn:x\"> <f1>aa</f1> <B> <f2>bb</f2> </B> </A>";
    const char *result = "<A xmlns=\"urn:x\"><f1>aa</f1><B><f2>bb</f2></B></A>";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, LYD_PRINT_SHRINK);

    lyd_free_all(target);
    lyd_free_all(source);
}

static void
test_container(void **state)
{
    const char *sch =
            "module A {\n"
            "    namespace \"aa:A\";\n"
            "    prefix A;\n"
            "    container A {\n"
            "        leaf f1 {type string;}\n"
            "        container B {\n"
            "            leaf f2 {type string;}\n"
            "        }\n"
            "        container C {\n"
            "            leaf f3 {type string;}\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg = "<A xmlns=\"aa:A\"> <B> <f2>aaa</f2> </B> </A>";
    const char *src = "<A xmlns=\"aa:A\"> <C> <f3>bbb</f3> </C> </A>";
    const char *result = "<A xmlns=\"aa:A\"><B><f2>aaa</f2></B><C><f3>bbb</f3></C></A>";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, LYD_PRINT_SHRINK);

    /* destroy */
    lyd_free_all(source);
    lyd_free_all(target);
}

static void
test_list(void **state)
{
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        list b-list1 {\n"
            "            key p1;\n"
            "            leaf p1 {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf p2 {\n"
            "                type string;\n"
            "            }\n"
            "            leaf p3 {\n"
            "                type boolean;\n"
            "                default false;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <p3>true</p3>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "    <p3>true</p3>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, 0);

    lyd_free_all(target);
    lyd_free_all(source);
}

static void
test_list2(void **state)
{
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        list b-list1 {\n"
            "            key p1;\n"
            "            leaf p1 {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf p2 {\n"
            "                type string;\n"
            "            }\n"
            "            container inner2 {\n"
            "                leaf p3 {\n"
            "                    type boolean;\n"
            "                    default false;\n"
            "                }\n"
            "                leaf p4 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, 0);

    lyd_free_all(source);
    lyd_free_all(target);
}

static void
test_dup_inst_list(void **state)
{
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        config false;\n"
            "        list b-list1 {\n"
            "            leaf p1 {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf p2 {\n"
            "                type string;\n"
            "            }\n"
            "            container inner2 {\n"
            "                leaf p4 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "  <b-list1>\n"
            "    <p1>2</p1>\n"
            "    <p2>a</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>b</p2>\n"
            "  </b-list1>\n"
            "  <b-list1>\n"
            "    <p1>1</p1>\n"
            "    <p2>a</p2>\n"
            "    <inner2>\n"
            "      <p4>val</p4>\n"
            "    </inner2>\n"
            "  </b-list1>\n"
            "  <b-list1>\n"
            "    <p1>2</p1>\n"
            "    <p2>a</p2>\n"
            "  </b-list1>\n"
            "</inner1>\n";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, 0);

    lyd_free_all(source);
    lyd_free_all(target);
}

static void
test_dup_inst_llist(void **state)
{
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "\n"
            "    container inner1 {\n"
            "        config false;\n"
            "        leaf-list b-llist1 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>b</b-llist1>\n"
            "  <b-llist1>c</b-llist1>\n"
            "  <b-llist1>d</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>b</b-llist1>\n"
            "  <b-llist1>c</b-llist1>\n"
            "  <b-llist1>d</b-llist1>\n"
            "</inner1>\n";
    const char *src =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-llist1>d</b-llist1>\n"
            "  <b-llist1>c</b-llist1>\n"
            "  <b-llist1>b</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>f</b-llist1>\n"
            "  <b-llist1>f</b-llist1>\n"
            "</inner1>\n";
    const char *result =
            "<inner1 xmlns=\"http://test/merge\">\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>b</b-llist1>\n"
            "  <b-llist1>c</b-llist1>\n"
            "  <b-llist1>d</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>b</b-llist1>\n"
            "  <b-llist1>c</b-llist1>\n"
            "  <b-llist1>d</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>a</b-llist1>\n"
            "  <b-llist1>f</b-llist1>\n"
            "  <b-llist1>f</b-llist1>\n"
            "</inner1>\n";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, 0);

    lyd_free_all(source);
    lyd_free_all(target);
}

static void
test_case(void **state)
{
    const char *sch =
            "module merge {\n"
            "    namespace \"http://test/merge\";\n"
            "    prefix merge;\n"
            "    container cont {\n"
            "        choice ch {\n"
            "            container inner {\n"
            "                leaf p1 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "            case c2 {\n"
            "                leaf p1 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n";

    const char *trg =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <inner>\n"
            "    <p1>1</p1>\n"
            "  </inner>\n"
            "</cont>\n";
    const char *src =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <p1>1</p1>\n"
            "</cont>\n";
    const char *result =
            "<cont xmlns=\"http://test/merge\">\n"
            "  <p1>1</p1>\n"
            "</cont>\n";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    /* merge them */
    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    /* check the result */
    LYD_TREE_CHECK_CHAR(target, result, 0);

    lyd_free_all(source);
    lyd_free_all(target);
}

static void
test_dflt(void **state)
{
    const char *sch =
            "module merge-dflt {\n"
            "    namespace \"urn:merge-dflt\";\n"
            "    prefix md;\n"
            "    container top {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "        leaf c {\n"
            "            type string;\n"
            "            default \"c_dflt\";\n"
            "        }\n"
            "    }\n"
            "}\n";
    struct lyd_node *target = NULL;
    struct lyd_node *source = NULL;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(lyd_new_path(NULL, UTEST_LYCTX, "/merge-dflt:top/c", "c_dflt", 0, &target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, UTEST_LYCTX, "/merge-dflt:top/a", "a_val", 0, &source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(source, UTEST_LYCTX, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&source, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&target, source, LYD_MERGE_DESTRUCT | LYD_MERGE_DEFAULTS), LY_SUCCESS);
    source = NULL;

    /* c should be replaced and now be default */
    assert_string_equal(lyd_child(target)->prev->schema->name, "c");
    assert_true(lyd_child(target)->prev->flags & LYD_DEFAULT);

    lyd_free_all(target);
    lyd_free_all(source);
}

static void
test_dflt2(void **state)
{
    const char *sch =
            "module merge-dflt {\n"
            "    namespace \"urn:merge-dflt\";\n"
            "    prefix md;\n"
            "    container top {\n"
            "        leaf a {\n"
            "            type string;\n"
            "        }\n"
            "        leaf b {\n"
            "            type string;\n"
            "        }\n"
            "        leaf c {\n"
            "            type string;\n"
            "            default \"c_dflt\";\n"
            "        }\n"
            "    }\n"
            "}\n";
    struct lyd_node *target;
    struct lyd_node *source;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(lyd_new_path(NULL, UTEST_LYCTX, "/merge-dflt:top/c", "c_dflt", 0, &target), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&target, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_new_path(NULL, UTEST_LYCTX, "/merge-dflt:top/a", "a_val", 0, &source), LY_SUCCESS);
    assert_int_equal(lyd_new_path(source, UTEST_LYCTX, "/merge-dflt:top/b", "b_val", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_validate_all(&source, NULL, LYD_VALIDATE_PRESENT, NULL), LY_SUCCESS);

    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);

    /* c should not be replaced, so c remains not default */
    assert_false(lyd_child(target)->flags & LYD_DEFAULT);

    lyd_free_all(target);
    lyd_free_all(source);
}

static void
test_leafrefs(void **state)
{
    const char *sch = "module x {"
            "  namespace urn:x;"
            "  prefix x;"
            "  list l {"
            "    key n;"
            "    leaf n { type string; }"
            "    leaf t { type string; }"
            "    leaf r { type leafref { path '/l/n'; } }}}";
    const char *trg = "<l xmlns=\"urn:x\"><n>a</n></l>"
            "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>";
    const char *src = "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>"
            "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>";
    const char *res = "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>"
            "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>"
            "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>";
    struct lyd_node *source, *target;

    UTEST_ADD_MODULE(sch, LYS_IN_YANG, NULL, NULL);

    LYD_TREE_CREATE(src, source);
    LYD_TREE_CREATE(trg, target);

    assert_int_equal(lyd_merge_siblings(&target, source, 0), LY_SUCCESS);

    LYD_TREE_CHECK_CHAR(target, res, LYD_PRINT_SHRINK);

    lyd_free_all(source);
    lyd_free_all(target);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_batch),
        UTEST(test_leaf),
        UTEST(test_container),
        UTEST(test_list),
        UTEST(test_list2),
        UTEST(test_dup_inst_list),
        UTEST(test_dup_inst_llist),
        UTEST(test_case),
        UTEST(test_dflt),
        UTEST(test_dflt2),
        UTEST(test_leafrefs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
