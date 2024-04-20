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

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

#define TEST_SUCCESS_PARSE_STORE_ONLY_XML(MOD_NAME, NODE_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STORE_ONLY, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 4, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_PARSE_STORE_ONLY_XML(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STORE_ONLY, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
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
test_data_basic_plugins_only_xml(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("a", "leaf l {type inet:ipv4-address;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* Stored via ipv4-address plugin */
    TEST_SUCCESS_XML("a", "l", "192.168.0.1", STRING, "192.168.0.1");
    TEST_ERROR_XML("a", "l", "192.168.0.333");
    TEST_ERROR_PARSE_STORE_ONLY_XML("a", "l", "192.168.0.333");

    /* Recreate context to get rid of all plugins */
    ly_ctx_destroy(UTEST_LYCTX);
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_BUILTIN_PLUGINS_ONLY, &UTEST_LYCTX));
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* Stored via string plugin */
    TEST_SUCCESS_XML("a", "l", "192.168.0.1", STRING, "192.168.0.1");
    TEST_ERROR_XML("a", "l", "192.168.0.333");
    CHECK_LOG_CTX("Unsatisfied pattern - \"192.168.0.333\" does not conform to \""
            "(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9]"
            "[0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])(%[\\p{N}\\p{L}]+)?\".", "/a:l", 1);
    TEST_SUCCESS_PARSE_STORE_ONLY_XML("a", "l", "192.168.0.333", STRING, "192.168.0.333");
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

static void
test_plugin_sort(void **state)
{
    const char *v1, *v2;
    const char *schema;
    struct lys_module *mod;
    struct lyd_value val1 = {0}, val2 = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_UNION]);
    struct lysc_type *lysc_type;
    struct ly_err_item *err = NULL;

    schema = MODULE_CREATE_YANG("a",
            "leaf l {type inet:ip-address;}"
            "leaf l2 {type inet:ipv6-address;}"
            "leaf l3 {type inet:ip-address-no-zone;}"
            "leaf l4 {type inet:ipv6-address-no-zone;}"
            "leaf l5 {type inet:ipv4-prefix;}"
            "leaf l6 {type inet:ipv6-prefix;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* ipv4-address */
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    v1 = "192.168.0.1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "192.168.0.2";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    v1 = "192.168.0.1%1A";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "192.168.0.1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* ipv6-address */
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data->next)->type;
    type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);

    v1 = "2008:15:0:0:0:0:feAC:1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "2008:15::feac:2";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    v1 = "FAAC:21:011:Da85::87:daaF%1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "FAAC:21:011:Da85::87:daaF%14";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* ipv4-address-no-zone */
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data->next->next)->type;
    type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_UNION]);
    v1 = "127.0.0.1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "127.0.1.1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* ipv6-address-no-zone */
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data->next->next->next)->type;
    type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_STRING]);
    v1 = "A:B:c:D:e:f:1:1";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "A:B:c:D:e:f:1:0";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* ipv4-prefix */
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data->next->next->next->next)->type;
    v1 = "0.1.58.4/32";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "0.1.58.4/16";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* ipv6-prefix */
    lysc_type = ((struct lysc_node_leaflist *)mod->compiled->data->next->next->next->next->next)->type;
    v1 = "::C:D:E:f:a/96";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v1, strlen(v1),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    v2 = "::C:D:E:f:a/112";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, v2, strlen(v2),
            0, LY_VALUE_JSON, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 < type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    assert_true(0 > type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
        UTEST(test_data_lyb),
        UTEST(test_plugin_sort),
        UTEST(test_data_basic_plugins_only_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
