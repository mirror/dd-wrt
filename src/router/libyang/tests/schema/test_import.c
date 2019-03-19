/**
 * \file test_import.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - module imports
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

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

static int
setup_ctx(void **state)
{
    *state = ly_ctx_new(NULL, 0);
    if (!*state) {
        return -1;
    }
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);
    return 0;
}

static void
test_mult_revisions(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *sch_yang = "module mod_a {"
        "namespace \"urn:cesnet:test:a\";"
        "prefix \"a\";"
        "import mod_r { prefix r1;  revision-date \"2016-06-19\";}"
        "import mod_r { prefix r2; }"
        "container data1 { uses r1:grp; }"
        "container data2 { uses r2:grp; }}";
    const char *sch_correct_yang = "module mod_a {"
        "namespace \"urn:cesnet:test:a\";"
        "prefix \"a\";"
        "import mod_r { prefix r1; revision-date \"2016-06-20\"; }"
        "import mod_r { prefix r2; }"
        "container data1 { uses r1:grp; }"
        "container data2 { uses r2:grp; }}";
    const char *sch_yin = "<module name=\"mod_a\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
        "xmlns:r1=\"urn:cesnet:test:r\" xmlns:r2=\"urn:cesnet:test:r\">"
        "<namespace uri=\"urn:cesnet:test:a\"/><prefix value=\"a\"/>"
        "<import module=\"mod_r\"><prefix value=\"r1\"/><revision-date date=\"2016-06-19\"/></import>"
        "<import module=\"mod_r\"><prefix value=\"r2\"/></import>"
        "<container name=\"data1\"><uses name=\"r1:grp\"/></container>"
        "<container name=\"data2\"><uses name=\"r2:grp\"/></container></module>";
    const char *sch_correct_yin = "<module name=\"mod_a\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
        "xmlns:r1=\"urn:cesnet:test:r\" xmlns:r2=\"urn:cesnet:test:r\">"
        "<namespace uri=\"urn:cesnet:test:a\"/><prefix value=\"a\"/>"
        "<import module=\"mod_r\"><prefix value=\"r1\"/><revision-date date=\"2016-06-20\"/></import>"
        "<import module=\"mod_r\"><prefix value=\"r2\"/></import>"
        "<container name=\"data1\"><uses name=\"r1:grp\"/></container>"
        "<container name=\"data2\"><uses name=\"r2:grp\"/></container></module>";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);
    assert_ptr_equal(lys_parse_mem(ctx, sch_yang, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, sch_correct_yang, LYS_IN_YANG), NULL);

    ly_ctx_destroy(*state, NULL);
    *state = ctx = ly_ctx_new(SCHEMA_FOLDER_YIN, 0);

    assert_ptr_equal(lys_parse_mem(ctx, sch_yin, LYS_IN_YIN), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, sch_correct_yin, LYS_IN_YIN), NULL);
}

static void
test_circular_import(void **state)
{
    struct ly_ctx *ctx = *state;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    assert_ptr_equal(ly_ctx_load_module(ctx, "circ_imp1", NULL), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_CIRC_IMPORTS);

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    assert_ptr_equal(ly_ctx_load_module(ctx, "circ_imp1", NULL), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_CIRC_IMPORTS);
}

/*
 * When an imported module is targeted in augment or leafref, it must be
 * (automatically) set to implemented module.
 */
static void
test_autoimplement_augment_import(void **state)
{
    struct ly_ctx *ctx = *state;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);
    assert_ptr_not_equal(ly_ctx_load_module(ctx, "impl_aug_a", NULL), NULL);
}

static void
test_autoimplement_leafref_import(void **state)
{
    struct ly_ctx *ctx = *state;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);
    assert_ptr_not_equal(ly_ctx_load_module(ctx, "impl_lr_a", NULL), NULL);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_mult_revisions, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_circular_import, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_autoimplement_augment_import, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_autoimplement_leafref_import, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
