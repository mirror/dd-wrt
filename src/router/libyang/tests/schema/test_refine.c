/**
 * \file test_refine.c
 * \author Frank Rimpler <frank.rimpler@adtran.com>
 * \brief libyang tests - refines
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cmocka.h>

#include "libyang.h"
#include "tests/config.h"

#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

#define MOD_COUNT 7
#define YANG_MOD_IDX(idx) (idx)

struct ly_ctx *ctx;

static int
setup_ctx_yang(void **state)
{
    *state = malloc(strlen(TESTS_DIR) + 40);
    assert_non_null(*state);
    memcpy(*state, SCHEMA_FOLDER_YANG, strlen(SCHEMA_FOLDER_YANG) + 1);

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    free(*state);
    ly_ctx_destroy(ctx, NULL);

    return 0;
}

/* this test verifies with the modules refgrp-m1..m4 the correct
   resolution of groupings/augments by node-id in a refine statement */
static void
test_refine(void **state)
{
    int length;
    char *path = *state;
    const struct lys_module *module;

    ly_ctx_set_searchdir(ctx, path);
    length = strlen(path);
    strcpy(path + length, "/refgrp-m1.yang");
    if (!(module = lys_parse_path(ctx, path, LYS_IN_YANG))) {
        fail();
    }
}

int
main(void)
{
    ly_verb(LY_LLWRN);
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_refine, setup_ctx_yang, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
