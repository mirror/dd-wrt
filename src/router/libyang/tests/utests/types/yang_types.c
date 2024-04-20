/**
 * @file yang_types.c
 * @author Michal Vaško <mvasko@cesnet.cz>
 * @brief test for ietf-yang-types values
 *
 * Copyright (c) 2021 - 2023 CESNET, z.s.p.o.
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

#include <stdlib.h>

#include "compat.h"
#include "libyang.h"

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
    "  import ietf-yang-types {\n" \
    "    prefix yang;\n" \
    "  }\n" \
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

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA, RET) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, RET, tree); \
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
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("a",
            "leaf l {type yang:date-and-time;}"
            "leaf l21 {type yang:hex-string;}"
            "leaf l22 {type yang:uuid;}"
            "leaf l3 {type yang:xpath1.0;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    schema = MODULE_CREATE_YANG("b",
            "");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* date-and-time */
    TEST_SUCCESS_XML("a", "l", "2005-05-25T23:15:15.88888Z", STRING, "2005-05-25T21:15:15.88888-02:00");
    TEST_SUCCESS_XML("a", "l", "2005-05-31T23:15:15-08:59", STRING, "2005-06-01T06:14:15-02:00");
    TEST_SUCCESS_XML("a", "l", "2005-05-31T23:15:15-23:00", STRING, "2005-06-01T20:15:15-02:00");

    /* test 1 second before epoch (mktime returns -1, but it is a correct value), with and without DST */
    TEST_SUCCESS_XML("a", "l", "1970-01-01T00:59:59-02:00", STRING, "1970-01-01T00:59:59-02:00");
    TEST_SUCCESS_XML("a", "l", "1969-12-31T23:59:59-02:00", STRING, "1969-12-31T23:59:59-02:00");

    /* canonize */
    TEST_SUCCESS_XML("a", "l", "2005-02-29T23:15:15-02:00", STRING, "2005-03-01T23:15:15-02:00");

    /* fractional hours */
    TEST_SUCCESS_XML("a", "l", "2005-05-25T23:15:15.88888+04:30", STRING, "2005-05-25T16:45:15.88888-02:00");

    /* unknown timezone -- timezone conversion MUST NOT happen */
    TEST_SUCCESS_XML("a", "l", "2017-02-01T00:00:00-00:00", STRING, "2017-02-01T00:00:00-00:00");
    TEST_SUCCESS_XML("a", "l", "2021-02-29T00:00:00-00:00", STRING, "2021-03-01T00:00:00-00:00");

    TEST_ERROR_XML("a", "l", "2005-05-31T23:15:15.-08:00", LY_EVALID);
    CHECK_LOG_CTX("Unsatisfied pattern - \"2005-05-31T23:15:15.-08:00\" does not conform to "
            "\"\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(\\.\\d+)?(Z|[\\+\\-]\\d{2}:\\d{2})\".",
            "/a:l", 1);

    TEST_ERROR_XML("a", "l", "2023-16-15T20:13:01+01:00", LY_EINVAL);
    CHECK_LOG_CTX("Invalid date-and-time month \"15\".", "/a:l", 1);

    TEST_ERROR_XML("a", "l", "2023-10-15T20:13:01+95:00", LY_EINVAL);
    CHECK_LOG_CTX("Invalid date-and-time timezone hour \"95\".", "/a:l", 1);

    /* hex-string */
    TEST_SUCCESS_XML("a", "l21", "DB:BA:12:54:fa", STRING, "db:ba:12:54:fa");
    TEST_SUCCESS_XML("a", "l22", "f81D4fAE-7dec-11d0-A765-00a0c91E6BF6", STRING, "f81d4fae-7dec-11d0-a765-00a0c91e6bf6");

    /* xpath1.0 */
    TEST_SUCCESS_XML("a\" xmlns:aa=\"urn:tests:a", "l3", "/aa:l3[. = '4']", STRING, "/a:l3[.='4']");
    TEST_SUCCESS_XML("a\" xmlns:yl=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\" "
            "xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores", "l3",
            "/yl:yang-library/yl:datastore/yl:name = 'ds:running'", STRING,
            "/ietf-yang-library:yang-library/datastore/name='ietf-datastores:running'");
    TEST_SUCCESS_XML("a\" xmlns:a1=\"urn:tests:a\" xmlns:a2=\"urn:tests:a\" xmlns:bb=\"urn:tests:b", "l3",
            "/a1:node1/a2:node2[a1:node3/bb:node4]/bb:node5 | bb:node6 and (bb:node7)", STRING,
            "/a:node1/node2[node3/b:node4]/b:node5 | b:node6 and (b:node7)");
    TEST_SUCCESS_XML("a", "l3", "/l3[. = '4']", STRING, "/l3[.='4']");

    TEST_ERROR_XML("a", "l3", "/a:l3[. = '4']", LY_EVALID);
    CHECK_LOG_CTX("Failed to resolve prefix \"a\".", "/a:l3", 1);
    TEST_ERROR_XML("a\" xmlns:yl=\"urn:ietf:params:xml:ns:yang:ietf-yang-library", "l3",
            "/yl:yang-library/yl:datastore/yl::name", LY_EVALID);
    CHECK_LOG_CTX("Storing value failed.", "/a:l3", 1);
    CHECK_LOG_CTX("Invalid character 'y'[31] of expression '/yl:yang-library/yl:datastore/yl::name'.", "/a:l3", 1);
}

static void
test_print(void **state)
{
    const char *schema = MODULE_CREATE_YANG("a", "leaf l {type yang:xpath1.0;}");
    const char *data, *expected;
    struct lyd_node *tree;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* XML data */
    data = "<l xmlns=\"urn:tests:a\" xmlns:aa=\"urn:tests:a\">/aa:l[. = '/aa:l']</l>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);

    /* XML print */
    expected = "<l xmlns=\"urn:tests:a\" xmlns:pref=\"urn:tests:a\">/pref:l[.='/pref:l']</l>";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    /* JSON print */
    expected = "{\"a:l\":\"/a:l[.='/a:l']\"}";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_tree(tree);

    /* JSON data */
    data = "{\"a:l\":\"/a:l/k/m[. = '/a:l']\"}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);

    /* XML print */
    expected = "<l xmlns=\"urn:tests:a\" xmlns:pref=\"urn:tests:a\">/pref:l/pref:k/pref:m[.='/pref:l']</l>";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    /* JSON print */
    expected = "{\"a:l\":\"/a:l/k/m[. = '/a:l']\"}";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_tree(tree);
}

static void
test_duplicate(void **state)
{
    const char *schema = MODULE_CREATE_YANG("a", "leaf l1 {type yang:date-and-time;} leaf l2 {type yang:xpath1.0;}");
    const char *data, *expected;
    struct lyd_node *tree, *dup;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    data = "<l1 xmlns=\"urn:tests:a\">2005-05-25T23:15:15.88888+04:30</l1>"
            "<l2 xmlns=\"urn:tests:a\" xmlns:aa=\"urn:tests:a\">/aa:l2[. = '/aa:l2']</l2>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);

    /* duplicate */
    assert_int_equal(LY_SUCCESS, lyd_dup_siblings(tree, NULL, 0, &dup));

    /* print */
    expected = "<l1 xmlns=\"urn:tests:a\">2005-05-25T16:45:15.88888-02:00</l1>"
            "<l2 xmlns=\"urn:tests:a\" xmlns:pref=\"urn:tests:a\">/pref:l2[.='/pref:l2']</l2>";
    CHECK_LYD_STRING_PARAM(dup, expected, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_siblings(tree);
    lyd_free_siblings(dup);
}

static void
test_lyb(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("a",
            "leaf l {type yang:date-and-time;}"
            "leaf l2 {type yang:xpath1.0;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* date-and-time */
    TEST_SUCCESS_LYB("a", "l", "2005-05-25T23:15:15.88888Z");
    TEST_SUCCESS_LYB("a", "l", "2005-05-31T23:15:15-08:59");
    TEST_SUCCESS_LYB("a", "l", "2005-05-01T20:15:15-00:00");

    /* xpath1.0 */
    TEST_SUCCESS_LYB("a\" xmlns:aa=\"urn:tests:a", "l2", "/aa:l2[. = '4']");
}

static void
test_sort(void **state)
{
    const char *v1, *v2;
    const char *schema;
    struct lys_module *mod;
    struct lyd_value val1 = {0}, val2 = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "ietf-yang-types", "2013-07-15", "date-and-time");
    struct lysc_type *lysc_type;
    struct ly_err_item *err = NULL;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("a",
            "leaf-list ll {type yang:date-and-time;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data)->type;

    /* v1 > v2, v2 < v1, v1 == v1 */
    v1 = "2005-05-25T23:15:15Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2005-05-25T23:15:14Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* unknown timezone */
    v1 = "2005-05-25T23:15:15Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2005-05-25T23:15:15-00:00";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val2));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* fractions of a second */
    v1 = "2005-05-25T23:15:15.88888Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2005-05-25T23:15:15.88889Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* zero fractions of a second */
    v1 = "2005-05-25T23:15:15.00Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2005-05-25T23:15:15Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* zero fractions of a second and non-zero */
    v1 = "2005-05-25T23:15:15.00Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2005-05-25T23:15:15.0001Z";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_int_equal(-1, type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(1, type->sort(UTEST_LYCTX, &val2, &val1));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_print),
        UTEST(test_duplicate),
        UTEST(test_lyb),
        UTEST(test_sort),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
