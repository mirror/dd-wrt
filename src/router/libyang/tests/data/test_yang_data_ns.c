/**
 * @file test_yang_data_ns.c
 * @author Peter Schoenmaker <pds@ntt.net>
 * @brief Cmocka tests for YANG data namespace.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
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
#include <string.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    const struct lys_module *mod1, *mod2, *mod_nc;
    struct lyd_node *dt;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *yang1 = "module yang-data-config-ns1 {\n"
        "  prefix dcn1;\n"
        "  namespace \"urn:cesnet:yang-data-config-ns1\";\n"
        "  container con1 {\n"
        "    leaf leaf1 {\n"
        "        type string;\n"
        "    }\n"
        "  }\n"
        "}";

    const char *yang2 = "module yang-data-config-ns2 {\n"
        "  prefix dcn2;\n"
        "  namespace \"urn:cesnet:yang-data-config-ns2\";\n"
        "  container con2 {\n"
        "    leaf leaf2 {\n"
        "        type string;\n"
        "    }\n"
        "  }\n"
        "}";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf/", 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    st->mod1 = lys_parse_mem(st->ctx, yang1, LYS_IN_YANG);
    if (!st->mod1){
        fprintf(stderr, "Failed to load module 1.\n");
        return -1;
    }
    st->mod2 = lys_parse_mem(st->ctx, yang2, LYS_IN_YANG);
    if (!st->mod2){
        fprintf(stderr, "Failed to load module 2.\n");
        return -1;
    }
    return 0;
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
test_anydata_ns(void **state)
{
    struct state *st = (*state);
    struct lyd_node *node1 = NULL, *node2 = NULL;

    char *s;
    const char *opentag = "<config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">";

    if(!(st->mod_nc = ly_ctx_get_module(st->ctx, "ietf-netconf", NULL, 0))) {
        st->mod_nc = ly_ctx_load_module(st->ctx, "ietf-netconf", NULL);
    }
    assert_ptr_not_equal(st->mod_nc, NULL);

    node1 = lyd_new(NULL, st->mod1, "con1");
    assert_ptr_not_equal(node1, NULL);
    
    assert_ptr_not_equal(lyd_insert_attr(node1, st->mod_nc, "operation", "create"), NULL);
  
    assert_ptr_not_equal(lyd_new_leaf(node1, st->mod1, "leaf1", "test1234"), NULL);
 
    node2 = lyd_new(NULL, st->mod2, "con2");
    assert_ptr_not_equal(node2, NULL);  
    assert_ptr_not_equal(lyd_insert_attr(node2, st->mod_nc, "operation", "merge"), NULL);

    assert_ptr_not_equal(lyd_new_leaf(node2, st->mod2, "leaf2", "leaf2-test1234"), NULL);
  
    assert_return_code(lyd_insert_sibling(&node1, node2), 0);

    st->dt = lyd_new_anydata(NULL, st->mod_nc, "config", node1, LYD_ANYDATA_DATATREE);
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&s, st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_FORMAT);
    assert_return_code(strncmp(s, opentag, strlen(opentag)), 0);
    free(s);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_anydata_ns, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
