/**
 * @file test_madnatory.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for mandatroy nodes in data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
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

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/mandatory.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }


    /* schema */
    if (!lys_parse_path(st->ctx, schemafile, LYS_IN_YIN)) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
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
test_mandatory(void **state)
{
    struct state *st = (*state);

    const char miss_leaf1[] = "<top xmlns=\"urn:libyang:tests:mandatory\"/>"
                              "<topleaf xmlns=\"urn:libyang:tests:mandatory\"/>";
    const char few_llist1[] = "<top xmlns=\"urn:libyang:tests:mandatory\"><leaf1>a</leaf1></top>";
    const char many_llist1[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                                 "<leaf1>a</leaf1>"
                                 "<llist1>1</llist1><llist1>2</llist1><llist1>3</llist1>"
                                 "<llist1>4</llist1><llist1>5</llist1><llist1>6</llist1>"
                               "</top>";
    const char miss_leaf2[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                                "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                              "</top>";
    const char miss_choice2[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                                  "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                                  "<cont1><cont2><cont3><leaf2>5</leaf2></cont3></cont2></cont1>"
                                  "<leaf3>b</leaf3>"
                                "</top>";
    const char miss_leaf6[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                                "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                                "<cont1><cont2><cont3><leaf2>5</leaf2></cont3></cont2></cont1>"
                              "</top>";
    const char miss_leaf7[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                                "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                                "<cont1><cont2><cont3><leaf2>5</leaf2></cont3></cont2></cont1>"
                                "<leaf3>c</leaf3><leaf5>d</leaf5><leaf6/>"
                              "</top>";
    const char miss_topleaf[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                           "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                           "<cont1><cont2><cont3><leaf2>5</leaf2></cont3></cont2></cont1>"
                           "<leaf3>c</leaf3><leaf5>d</leaf5><leaf6/><leaf7/>"
                         "</top>";
    const char valid[] = "<top xmlns=\"urn:libyang:tests:mandatory\">"
                           "<leaf1>a</leaf1><llist1>1</llist1><llist1>2</llist1>"
                           "<cont1><cont2><cont3><leaf2>5</leaf2></cont3></cont2></cont1>"
                           "<leaf3>c</leaf3><leaf5>d</leaf5><leaf6/><leaf7/>"
                         "</top><topleaf xmlns=\"urn:libyang:tests:mandatory\"/>";

    st->dt = lyd_parse_mem(st->ctx, miss_leaf1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSELEM);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top");

    st->dt = lyd_parse_mem(st->ctx, few_llist1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOMIN);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top");

    st->dt = lyd_parse_mem(st->ctx, many_llist1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOMAX);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top/llist1[.='6']");

    st->dt = lyd_parse_mem(st->ctx, miss_leaf2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSELEM);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top/cont1/cont2/cont3");

    st->dt = lyd_parse_mem(st->ctx, miss_choice2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOMANDCHOICE);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top");

    st->dt = lyd_parse_mem(st->ctx, miss_leaf6, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSELEM);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top");

    st->dt = lyd_parse_mem(st->ctx, miss_leaf7, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSELEM);
    assert_string_equal(ly_errpath(st->ctx), "/mandatory:top");

    st->dt = lyd_parse_mem(st->ctx, miss_topleaf, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_MISSELEM);
    assert_string_equal(ly_errpath(st->ctx), "/");

    st->dt = lyd_parse_mem(st->ctx, valid, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_SUCCESS);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_mandatory, setup_f, teardown_f)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
