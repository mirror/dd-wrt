/**
 * \file test_deviation.c
 * \author Frank Rimpler <frank.rimpler@adtran.com>
 * \brief libyang tests - deviations
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

struct ly_ctx *ctx;

static int
setup_ctx_yang(void **state)
{
    (void)state;

    ctx = ly_ctx_new(SCHEMA_FOLDER_YANG, 0);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    (void)state;

    ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_deviation(void **state)
{
    (void)state;
    char *str;
    const struct lys_module *module;

    if (!(module = ly_ctx_load_module(ctx, "deviation1-dv", NULL))) {
        fail();
    }
    lys_print_mem(&str, module, LYS_OUT_YANG, NULL, 0, 0);
    free(str);
    assert_int_equal(ly_errno, 0);
}

static void
test_augment_deviation(void **state)
{
    (void)state;
    char *str;
    const struct lys_module *mod;

    if (!ly_ctx_load_module(ctx, "z-dev", NULL)) {
        fail();
    }
    mod = ly_ctx_get_module(ctx, "z", NULL, 0);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&str, mod, LYS_YANG, NULL, 0, 0);
    free(str);
    assert_int_equal(ly_errno, 0);
}

int
main(void)
{
    ly_verb(LY_LLWRN);
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_deviation, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_augment_deviation, setup_ctx_yang, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
