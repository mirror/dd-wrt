/**
 * @file test_sec6_1_3.c
 * @author Pavol Vican
 * @brief Cmocka test for RFC 6020 section 6.1.3 conformance.
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
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <cmocka.h>
#include <string.h>

#include "tests/config.h"
#include "libyang.h"

#define TEST_DIR "sec6_1_3"
#define TEST_NAME test_sec6_1_3
#define TEST_SCHEMA_FORMAT LYS_YANG
#define TEST_SCHEMA_COUNT 2

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *node;
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
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free(st->node);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
TEST_QUOTING(void **state)
{
    struct state *st = (*state);
    char buf[1024];
    const struct lys_module *mod;
    char *dsc="Test for special characters: { } ; space /* multiple\nline comment */ // comment";
    char *ref="Test for special characters: { } ; space /* multiple line comment */ // comment";
    char *contact="\"\" \\\\ \\  \n\t  \\n\\t ";

    sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod1.yang");
    mod = lys_parse_path(st->ctx, buf, TEST_SCHEMA_FORMAT);
    assert_ptr_not_equal(mod, NULL);
    assert_string_equal(mod->dsc, dsc);
    assert_string_equal(mod->ref, ref);
    assert_string_equal(mod->org, dsc);
    assert_string_equal(mod->contact, contact);
}

static void
TEST_ESCAPE_CHARACTER_IN_DOUBLE_QUOTING(void **state)
{
    struct state *st = (*state);
    char buf[1024];
    const struct lys_module *mod;
    char *dsc="a\n b";
    char *ref="a\t\n\tb";
    char *org="a\t  \t\n  \t\tb";
    char *contact="a  \t\t\n\tb";

    sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod5.yang");
    mod = lys_parse_path(st->ctx, buf, TEST_SCHEMA_FORMAT);
    assert_ptr_not_equal(mod, NULL);
    assert_string_equal(mod->dsc, dsc);
    assert_string_equal(mod->ref, ref);
    assert_string_equal(mod->org, org);
    assert_string_equal(mod->contact, contact);
}

static void
TEST_DOUBLE_QUOTING(void **state)
{
    struct state *st = (*state);
    char buf[1024];
    const struct lys_module *mod;
    char *pattern="This is example text,\nwhich is formatted.";

    sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod2.yang");
    mod = lys_parse_path(st->ctx, buf, TEST_SCHEMA_FORMAT);
    assert_ptr_not_equal(mod, NULL);
    assert_string_equal(mod->dsc, pattern);
    assert_string_equal(mod->ref, pattern);
    assert_string_equal(mod->org, pattern);
    assert_string_equal(mod->contact, pattern);
}

static void
TEST_ILLEGAL_STRING(void **state)
{
    struct state *st = (*state);
    char buf[1024];
    const struct lys_module *mod;
    int i;

    for(i=0; i<TEST_SCHEMA_COUNT; i++) {
        sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod%d.yang",i+3);
        mod = lys_parse_path(st->ctx, buf, TEST_SCHEMA_FORMAT);
        assert_ptr_equal(mod, NULL);
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(TEST_QUOTING, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(TEST_ESCAPE_CHARACTER_IN_DOUBLE_QUOTING, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(TEST_DOUBLE_QUOTING, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(TEST_ILLEGAL_STRING, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

