/**
 * @file test_defaults.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for processing default values.
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
    const struct lys_module *mod;
    struct lyd_node *dt;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/unique.yin";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schemas */
    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_un_correct(void **state)
{
    struct state *st = (*state);
    const char *xml = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><value>1</value><a>1</a><input><x>1</x><y>1</y></input></list>"
                        "<list><name>y</name><value>2</value><a>2</a><input><x>2</x><y>2</y></input></list>"
                      "</un>";

    st->dt = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_un_defaults(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><value>1</value><input><y>1</y></input></list>"
                        "<list><name>y</name><value>2</value><input><y>2</y></input></list>"
                       "</un>";
    const char *xml2 = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><a>1</a><input><x>1</x></input></list>"
                        "<list><name>y</name><a>2</a><input><x>1</x></input></list>"
                       "</un>";
    const char *xml3 = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><input><y>1</y></input></list>"
                        "<list><name>y</name><input><y>2</y></input></list>"
                       "</un>";

    st->dt = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_free(st->dt);

    st->dt = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_free(st->dt);

    st->dt = lyd_parse_mem(st->ctx, xml3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(st->dt, NULL);
    assert_int_equal(ly_errno, LY_EVALID);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOUNIQ);
    assert_string_equal(ly_errmsg(st->ctx), "Unique data leaf(s) \"value a\" not satisfied in \"/unique:un/list[name='x']\" and \"/unique:un/list[name='y']\".");
}

static void
test_un_empty(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><b>1</b><input><z>1</z></input></list>"
                        "<list><name>y</name><b>1</b><input><z>1</z></input></list>"
                       "</un>";
    const char *xml2 = "<un xmlns=\"urn:libyang:tests:unique\">"
                        "<list><name>x</name><a>1</a></list>"
                        "<list><name>y</name><a>2</a></list>"
                       "</un>";

    st->dt = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_free(st->dt);

    st->dt = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_schema_inpath(void **state)
{
    struct state *st = (*state);
    const char *sch = "module unique2 {"
                      "  namespace \"urn:libyang:tests:unique2\";"
                      "  prefix un2;"
                      "  list list1 {"
                      "    key key;"
                      "    leaf key { type string; }"
                      "    unique sublist/value;"
                      "    list sublist {"
                      "      key key;"
                      "      leaf key { type string; }"
                      "      leaf value { type string; }"
                      "} } }";

    assert_ptr_equal(lys_parse_mem(st->ctx, sch, LYS_IN_YANG), NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_un_correct, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_un_defaults, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_un_empty, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_schema_inpath, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
