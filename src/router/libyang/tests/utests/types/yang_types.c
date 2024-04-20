/**
 * @file yang_types.c
 * @author Michal Vaško <mvasko@cesnet.cz>
 * @brief test for ietf-yang-types values
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

#include <stdlib.h>

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
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, ## __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_data_xml(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("a",
            "leaf l {type yang:date-and-time;}"
            "leaf l2 {type yang:xpath1.0;}");
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

    TEST_ERROR_XML("a", "l", "2005-05-31T23:15:15.-08:00");
    CHECK_LOG_CTX("Unsatisfied pattern - \"2005-05-31T23:15:15.-08:00\" does not conform to "
            "\"\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(\\.\\d+)?(Z|[\\+\\-]\\d{2}:\\d{2})\".",
            "Schema location /a:l, line number 1.");

    /* xpath1.0 */
    TEST_SUCCESS_XML("a\" xmlns:aa=\"urn:tests:a", "l2", "/aa:l2[. = '4']", STRING, "/a:l2[. = '4']");
    TEST_SUCCESS_XML("a\" xmlns:yl=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\" "
            "xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores", "l2",
            "/yl:yang-library/yl:datastore/yl:name = 'ds:running'", STRING,
            "/ietf-yang-library:yang-library/ietf-yang-library:datastore/ietf-yang-library:name = 'ietf-datastores:running'");
    TEST_SUCCESS_XML("a", "l2", "/l2[. = '4']", STRING, "/l2[. = '4']");

    TEST_ERROR_XML("a", "l2", "/a:l2[. = '4']");
    CHECK_LOG_CTX("Failed to resolve prefix \"a\".", "Schema location /a:l2, line number 1.");
    TEST_ERROR_XML("a\" xmlns:yl=\"urn:ietf:params:xml:ns:yang:ietf-yang-library", "l2",
            "/yl:yang-library/yl:datastore/yl::name");
    CHECK_LOG_CTX("Storing value \"/yl:yang-library/yl:datastore/yl::name\" failed.", "Schema location /a:l2, line number 1.",
            "Invalid character ':'[34] of expression '/yl:yang-library/yl:datastore/yl::name'.",
            "Schema location /a:l2, line number 1.");
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
    expected = "<l xmlns=\"urn:tests:a\" xmlns:pref=\"urn:tests:a\">/pref:l[. = '/pref:l']</l>";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    /* JSON print */
    expected = "{\"a:l\":\"/a:l[. = '/a:l']\"}";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_tree(tree);

    /* JSON data */
    data = "{\"a:l\":\"/a:l/k/m[. = '/a:l']\"}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);

    /* XML print */
    expected = "<l xmlns=\"urn:tests:a\" xmlns:pref=\"urn:tests:a\">/pref:l/pref:k/pref:m[. = '/pref:l']</l>";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    /* JSON print */
    expected = "{\"a:l\":\"/a:l/k/m[. = '/a:l']\"}";
    CHECK_LYD_STRING_PARAM(tree, expected, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_tree(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_print),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
