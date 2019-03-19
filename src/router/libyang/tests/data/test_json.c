/**
 * @file test_xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for XPath expression evaluation.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
};

static const char *if_data =
"{"
  "\"ietf-interfaces:interfaces\": {"
    "\"interface\": ["
      "{"
        "\"name\": \"iface1\","
        "\"description\": \"iface1 dsc\","
        "\"type\": \"iana-if-type:ethernetCsmacd\","
        "\"@type\": {"
          "\"yang:type_attr\":\"1\""
        "},"
        "\"enabled\": true,"
        "\"link-up-down-trap-enable\": \"disabled\","
        "\"ietf-ip:ipv4\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"14\""
          "},"
          "\"enabled\": true,"
          "\"forwarding\": true,"
          "\"mtu\": 68,"
          "\"address\": ["
            "{"
              "\"ip\": \"10.0.0.1\","
              "\"netmask\": \"255.0.0.0\""
            "},"
            "{"
              "\"ip\": \"172.0.0.1\","
              "\"prefix-length\": 16"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"10.0.0.2\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:f0\""
            "}"
          "]"
        "},"
        "\"ietf-ip:ipv6\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"16\""
          "},"
          "\"enabled\": true,"
          "\"forwarding\": false,"
          "\"mtu\": 1280,"
          "\"address\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:1\","
              "\"prefix-length\": 64"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:2\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:f0\""
            "}"
          "],"
          "\"dup-addr-detect-transmits\": 52,"
          "\"autoconf\": {"
            "\"create-global-addresses\": true,"
            "\"create-temporary-addresses\": false,"
            "\"temporary-valid-lifetime\": 600,"
            "\"temporary-preferred-lifetime\": 300"
          "}"
        "}"
      "},"
      "{"
        "\"name\": \"iface2\","
        "\"description\": \"iface2 dsc\","
        "\"type\": \"iana-if-type:softwareLoopback\","
        "\"@type\": {"
          "\"yang:type_attr\":\"2\""
        "},"
        "\"enabled\": false,"
        "\"link-up-down-trap-enable\": \"disabled\","
        "\"ietf-ip:ipv4\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"24\""
          "},"
          "\"address\": ["
            "{"
              "\"ip\": \"10.0.0.5\","
              "\"netmask\": \"255.0.0.0\""
            "},"
            "{"
              "\"ip\": \"172.0.0.5\","
              "\"prefix-length\": 16"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"10.0.0.1\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:fa\""
            "}"
          "]"
        "},"
        "\"ietf-ip:ipv6\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"26\""
          "},"
          "\"address\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:5\","
              "\"prefix-length\": 64"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:1\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:fa\""
            "}"
          "],"
          "\"dup-addr-detect-transmits\": 100,"
          "\"autoconf\": {"
            "\"create-global-addresses\": true,"
            "\"create-temporary-addresses\": false,"
            "\"temporary-valid-lifetime\": 600,"
            "\"temporary-preferred-lifetime\": 300"
          "}"
        "}"
      "}"
    "]"
  "}"
"}"
;

static const char *num_data =
"{"
  "\"numbers:nums\": {"
    "\"num1\": 9223372036854775807,"
    "\"num2\": 18446744073709551615,"
    "\"num3\": -2147483648,"
    "\"num4\": 4294967295,"
    "\"num5\": 9.87654321e+4,"
    "\"num6\": 987654321098765E-10,"
    "\"num7\": -922337203685477580.8,"
    "\"num8\": 922337203685477580.7,"
    "\"num9\": -9.223372036854775808,"
    "\"num10\": 9.223372036854775807,"
    "\"num11\": -92233720368.54775808e-10,"
    "\"num12\": 92233720.36854775807e10"
  "}"
"}"
;

static int
setup_f(struct state **state, const char *search_dir, const char **modules, int module_count)
{
    const struct lys_module *mod;
    int i;

    (*state) = calloc(1, sizeof **state);
    if (!(*state)) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    (*state)->ctx = ly_ctx_new(search_dir, 0);
    if (!(*state)->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schemas */
    for (i = 0; i < module_count; ++i) {
        mod = ly_ctx_load_module((*state)->ctx, modules[i], NULL);
        if (!mod) {
            fprintf(stderr, "Failed to load data module \"%s\".\n", modules[i]);
            goto error;
        }
        lys_features_enable(mod, "*");
    }

    return 0;

error:
    ly_ctx_destroy((*state)->ctx, NULL);
    free(*state);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_parse_if(void **state)
{
    struct state *st;
    const char *modules[] = {"ietf-interfaces", "ietf-ip", "iana-if-type"};
    int module_count = 3;

    if (setup_f(&st, TESTS_DIR "/schema/yin/ietf", modules, module_count)) {
        fail();
    }

    (*state) = st;

    st->dt = lyd_parse_mem(st->ctx, if_data, LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_parse_numbers(void **state)
{
    struct lyd_node_leaf_list *leaf;
    struct state *st;
    const char *modules[] = {"numbers"};
    int module_count = 1;

    if (setup_f(&st, TESTS_DIR "/data/files", modules, module_count)) {
        fail();
    }

    (*state) = st;

    st->dt = lyd_parse_mem(st->ctx, num_data, LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);

    /* num1 */
    leaf = (struct lyd_node_leaf_list *)st->dt->child;

    /* num2 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num3 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num4 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num5 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num6 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num7 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num8 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num9 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num10 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num11 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

    /* num12 */
    leaf = (struct lyd_node_leaf_list *)leaf->next;

}

int
main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_teardown(test_parse_if, teardown_f),
                    cmocka_unit_test_teardown(test_parse_numbers, teardown_f),
                    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

