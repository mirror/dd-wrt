/**
 * @file test_tree_data_dup.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for complex data duplications.
 *
 * Copyright (c) 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx1;
    struct ly_ctx *ctx2;
    struct lyd_node *dt1;
    struct lyd_node *dt2;
    struct lyd_node *dt3;
};
static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx1 = ly_ctx_new(NULL, 0);
    st->ctx2 = ly_ctx_new(NULL, 0);
    if (!st->ctx1 || !st->ctx2) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->dt1);
    lyd_free(st->dt2);
    lyd_free(st->dt3);
    ly_ctx_destroy(st->ctx1, NULL);
    ly_ctx_destroy(st->ctx2, NULL);

    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_dup_to_ctx(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }}";
    const char *data = "<x xmlns=\"urn:x\">hello</x>";

    /* case 1 - schema is only in the first context, duplicating data into the second context is supposed to
     *          fail because of missing schema */
    mod = lys_parse_mem(st->ctx1, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->dt1 = lyd_parse_mem(st->ctx1, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    st->dt2 = lyd_dup_to_ctx(st->dt1, 1, st->ctx2);
    assert_ptr_equal(st->dt2, NULL);
    assert_int_equal(ly_errno, LY_EINVAL);
    assert_string_equal(ly_errmsg(st->ctx2),
                        "Target context does not contain schema node for the data node being duplicated (x:x).");

    /* case 2 - with the schema present in both contexts, duplication should succeed */
    mod = lys_parse_mem(st->ctx2, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->dt2 = lyd_dup_to_ctx(st->dt1, 1, st->ctx2);
    assert_ptr_not_equal(st->dt2, NULL);
    /* the values are the same, but they are stored in different contexts */
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt1)->value_str,
                        ((struct lyd_node_leaf_list *)st->dt2)->value_str);
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1)->value_str,
                        ((struct lyd_node_leaf_list *)st->dt2)->value_str);
    /* and the schema nodes are the same, but comes from a different contexts */
    assert_int_equal(st->dt1->schema->nodetype, st->dt2->schema->nodetype);
    assert_string_equal(st->dt1->schema->name, st->dt2->schema->name);
    assert_string_equal(st->dt1->schema->module->name, st->dt2->schema->module->name);
    assert_ptr_equal(st->dt1->schema->module->ctx, st->ctx1);
    assert_ptr_equal(st->dt2->schema->module->ctx, st->ctx2);
}

static void
test_dup_to_ctx_bits(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  typedef mybits { type bits {"
                    "    bit disable;"
                    "    bit enable; } }"
                    "  leaf x { type mybits; }}";
    const char *data = "<x xmlns=\"urn:x\">enable</x>";
    char *printed = NULL;

    mod = lys_parse_mem(st->ctx1, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx2, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->dt1 = lyd_parse_mem(st->ctx1, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    st->dt2 = lyd_dup_to_ctx(st->dt1, 1, st->ctx2);
    assert_ptr_not_equal(st->dt2, NULL);
    /* the values are the same, but they are stored in different contexts */
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt1)->value_str,
                        ((struct lyd_node_leaf_list *)st->dt2)->value_str);
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1)->value_str,
                         ((struct lyd_node_leaf_list *)st->dt2)->value_str);
    /* check the value data */
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1)->value.bit,
                         ((struct lyd_node_leaf_list *)st->dt2)->value.bit);
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1)->value.bit[1],
                         ((struct lyd_node_leaf_list *)st->dt2)->value.bit[1]);
    /* first bit is not set, so the value pointer is NULL in both cases */
    assert_ptr_equal(((struct lyd_node_leaf_list *)st->dt1)->value.bit[0], NULL);
    assert_ptr_equal(((struct lyd_node_leaf_list *)st->dt2)->value.bit[0], NULL);
    /* and the schema nodes are the same, but comes from a different contexts */
    assert_int_equal(st->dt1->schema->nodetype, st->dt2->schema->nodetype);
    assert_string_equal(st->dt1->schema->name, st->dt2->schema->name);
    assert_string_equal(st->dt1->schema->module->name, st->dt2->schema->module->name);
    assert_ptr_equal(st->dt1->schema->module->ctx, st->ctx1);
    assert_ptr_equal(st->dt2->schema->module->ctx, st->ctx2);

    /* valgrind test - remove the first context and the access the duplicated data
     *                 supposed to be in the second context */
    lyd_free(st->dt1);
    ly_ctx_destroy(st->ctx1, NULL);
    st->dt1 = NULL;
    st->ctx1 = NULL;

    lyd_print_mem(&printed, st->dt2, LYD_XML, 0);
    assert_string_equal(printed, data);

    free(printed);
}

static void
test_dup_to_ctx_leafrefs(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  container x {"
                    "    leaf a { type string; }"
                    "    leaf b { type leafref { path ../a; } } } }";
    const char *data = "<x xmlns=\"urn:x\"><b>hello</b><a>hello</a></x>";
    char *printed = NULL;

    mod = lys_parse_mem(st->ctx1, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
    mod = lys_parse_mem(st->ctx2, sch, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->dt1 = lyd_parse_mem(st->ctx1, data, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    st->dt2 = lyd_dup_to_ctx(st->dt1, 1, st->ctx2);
    assert_ptr_not_equal(st->dt2, NULL);

    /* the result is not valid - the leafref is not resolved */
    assert_int_not_equal(((struct lyd_node_leaf_list *)st->dt2->child)->value_type, LY_TYPE_LEAFREF);
    assert_int_equal(lyd_validate(&st->dt2, LYD_OPT_CONFIG, st->ctx2), 0);
    assert_ptr_equal(((struct lyd_node_leaf_list *)st->dt2->child)->value_type, LY_TYPE_LEAFREF);

    /* the values are the same, but they are stored in different contexts */
    assert_string_equal(((struct lyd_node_leaf_list *)st->dt1->child)->value_str,
                        ((struct lyd_node_leaf_list *)st->dt2->child)->value_str);
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1->child)->value_str,
                         ((struct lyd_node_leaf_list *)st->dt2->child)->value_str);
    /* check the value data */
    assert_ptr_not_equal(((struct lyd_node_leaf_list *)st->dt1->child)->value.leafref,
                         ((struct lyd_node_leaf_list *)st->dt2->child)->value.leafref);
    /* and the schema nodes are the same, but comes from a different contexts */
    assert_int_equal(st->dt1->child->schema->nodetype, st->dt2->child->schema->nodetype);
    assert_string_equal(st->dt1->child->schema->name, st->dt2->child->schema->name);
    assert_string_equal(st->dt1->child->schema->module->name, st->dt2->child->schema->module->name);
    assert_ptr_equal(st->dt1->child->schema->module->ctx, st->ctx1);
    assert_ptr_equal(st->dt2->child->schema->module->ctx, st->ctx2);

    /* valgrind test - remove the first context and the access the duplicated data
     *                 supposed to be in the second context */
    lyd_free(st->dt1);
    ly_ctx_destroy(st->ctx1, NULL);
    st->dt1 = NULL;
    st->ctx1 = NULL;

    lyd_print_mem(&printed, st->dt2, LYD_XML, 0);
    assert_string_equal(printed, data);

    free(printed);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_dup_to_ctx, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_dup_to_ctx_bits, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_dup_to_ctx_leafrefs, setup_f, teardown_f),};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
