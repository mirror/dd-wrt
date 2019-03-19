/**
 * @file test_sec6_1_1.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka test for RFC 6020 section 6.1.1. conformance.
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

#define TEST_NAME test_sec6_1_1
#define TEST_SCHEMA "sec6_1_1/mod.yang"
#define TEST_SCHEMA_FORMAT LYS_YANG
#define TEST_SCHEMA_LOAD_FAIL 0
#define TEST_DATA ""
#define TEST_DATA_LOAD_FAIL 0

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
TEST_NAME(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, TESTS_DIR "/conformance/" TEST_SCHEMA, TEST_SCHEMA_FORMAT);
    if (TEST_SCHEMA_LOAD_FAIL) {
        assert_ptr_equal(mod, NULL);
    } else {
        assert_ptr_not_equal(mod, NULL);
    }

    if (TEST_DATA[0]) {
        st->node = lyd_parse_path(st->ctx, TESTS_DIR "/conformance/" TEST_DATA, LYD_XML, LYD_OPT_CONFIG);
        if (TEST_DATA_LOAD_FAIL) {
            assert_ptr_not_equal(st->node, NULL);
        } else {
            assert_ptr_equal(st->node, NULL);
        }
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(TEST_NAME, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

