/**
 * @file inet_types.c
 * @author Michal Vaško <mvasko@cesnet.cz>
 * @brief test for ietf-inet-types values
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
    "  import ietf-inet-types {\n" \
    "    prefix inet;\n" \
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
            "leaf l {type inet:ip-address;}"
            "leaf l2 {type inet:ipv6-address;}"
            "leaf l3 {type inet:ip-address-no-zone;}"
            "leaf l4 {type inet:ipv6-address-no-zone;}"
            "leaf l5 {type inet:ip-prefix;}"
            "leaf l6 {type inet:ipv4-prefix;}"
            "leaf l7 {type inet:ipv6-prefix;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* ip-address */
    TEST_SUCCESS_XML("a", "l", "192.168.0.1", UNION, "192.168.0.1", STRING, "192.168.0.1");
    TEST_SUCCESS_XML("a", "l", "192.168.0.1%12", UNION, "192.168.0.1%12", STRING, "192.168.0.1%12");
    TEST_SUCCESS_XML("a", "l", "2008:15:0:0:0:0:feAC:1", UNION, "2008:15::feac:1", STRING, "2008:15::feac:1");

    /* ipv6-address */
    TEST_SUCCESS_XML("a", "l2", "FAAC:21:011:Da85::87:daaF%1", STRING, "faac:21:11:da85::87:daaf%1");

    /* ip-address-no-zone */
    TEST_SUCCESS_XML("a", "l3", "127.0.0.1", UNION, "127.0.0.1", STRING, "127.0.0.1");
    TEST_SUCCESS_XML("a", "l3", "0:00:000:0000:000:00:0:1", UNION, "::1", STRING, "::1");

    /* ipv6-address-no-zone */
    TEST_SUCCESS_XML("a", "l4", "A:B:c:D:e:f:1:0", STRING, "a:b:c:d:e:f:1:0");

    /* ip-prefix */
    TEST_SUCCESS_XML("a", "l5", "158.1.58.4/1", UNION, "128.0.0.0/1", STRING, "128.0.0.0/1");
    TEST_SUCCESS_XML("a", "l5", "158.1.58.4/24", UNION, "158.1.58.0/24", STRING, "158.1.58.0/24");
    TEST_SUCCESS_XML("a", "l5", "2000:A:B:C:D:E:f:a/16", UNION, "2000::/16", STRING, "2000::/16");

    /* ipv4-prefix */
    TEST_SUCCESS_XML("a", "l6", "0.1.58.4/32", STRING, "0.1.58.4/32");
    TEST_SUCCESS_XML("a", "l6", "12.1.58.4/8", STRING, "12.0.0.0/8");

    /* ipv6-prefix */
    TEST_SUCCESS_XML("a", "l7", "::C:D:E:f:a/112", STRING, "::c:d:e:f:0/112");
    TEST_SUCCESS_XML("a", "l7", "::C:D:E:f:a/110", STRING, "::c:d:e:c:0/110");
    TEST_SUCCESS_XML("a", "l7", "::C:D:E:f:a/96", STRING, "::c:d:e:0:0/96");
    TEST_SUCCESS_XML("a", "l7", "::C:D:E:f:a/55", STRING, "::/55");
}

static void
test_data_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb",
            "leaf l {type inet:ip-address;}"
            "leaf l2 {type inet:ipv6-address;}"
            "leaf l3 {type inet:ip-address-no-zone;}"
            "leaf l4 {type inet:ipv6-address-no-zone;}"
            "leaf l5 {type inet:ip-prefix;}"
            "leaf l6 {type inet:ipv4-prefix;}"
            "leaf l7 {type inet:ipv6-prefix;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_LYB("lyb", "l", "192.168.0.1");
    TEST_SUCCESS_LYB("lyb", "l2", "FAAC:21:011:Da85::87:daaF%1");
    TEST_SUCCESS_LYB("lyb", "l3", "127.0.0.1");
    TEST_SUCCESS_LYB("lyb", "l4", "A:B:c:D:e:f:1:0");
    TEST_SUCCESS_LYB("lyb", "l5", "158.1.58.4/1");
    TEST_SUCCESS_LYB("lyb", "l6", "12.1.58.4/8");
    TEST_SUCCESS_LYB("lyb", "l7", "::C:D:E:f:a/112");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_data_lyb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
