/*
 * \file test_feature.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - features and if-features
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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
setup_ctx_yin(void **state)
{
    struct ly_ctx *ctx;

    ctx = ly_ctx_new(SCHEMA_FOLDER_YIN, 0);
    assert_non_null(ctx);

    *state = ctx;
    return 0;
}

static int
setup_ctx_yang(void **state)
{
    struct ly_ctx *ctx;

    ctx = ly_ctx_new(SCHEMA_FOLDER_YANG, 0);
    assert_non_null(ctx);

    *state = ctx;
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);

    return 0;
}

static void
test_fullset(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *mod;
    char *buf = NULL;

    const char *tree_alldisabled = "module: features\n"
"  +--rw lst* [id] {not a}?\n"
"  |  +--rw id    string\n"
"  +--rw (ch)? {not (a and b)}?\n"
"  |  +--:(ch3)\n"
"  |     +--rw ch3?   string\n"
"  +--rw axml?   anyxml {not (a or b)}?\n";

    const char *tree_a = "module: features\n"
"  +--rw grp?    string\n"
"  +--rw cont! {a}?\n"
"  +--rw ll*     string {a or b}?\n"
"  +--rw (ch)? {not (a and b)}?\n"
"     +--:(ch1) {a}?\n"
"     |  +--rw ch1?   string\n"
"     +--:(ch3)\n"
"        +--rw ch3?   string\n";

    const char *tree_ab = "module: features\n"
"  +--rw grp?    string\n"
"  +--rw cont! {a}?\n"
"  +--rw lf?     string {a and b}?\n"
"  +--rw ll*     string {a or b}?\n";

    const char *tree_abaa = "module: features\n"
"  +--rw grp?    string\n"
"  +--rw cont! {a}?\n"
"  |  +--rw aug?   string {aa}?\n"
"  +--rw lf?     string {a and b}?\n"
"  +--rw ll*     string {a or b}?\n\n"
"  rpcs:\n"
"    +---x rpc1 {aa}?\n\n"
"  notifications:\n"
"    +---n notif1 {aa}?\n";

    const char *tree_b = "module: features\n"
"  +--rw ll*     string {a or b}?\n"
"  +--rw lst* [id] {not a}?\n"
"  |  +--rw id    string\n"
"  +--rw (ch)? {not (a and b)}?\n"
"     +--:(ch2) {b}?\n"
"     |  +--rw ch2?   string\n"
"     +--:(ch3)\n"
"        +--rw ch3?   string\n";

    mod = ly_ctx_load_module(ctx, "features", NULL);
    assert_non_null(mod);

    lys_print_mem(&buf, mod, LYS_OUT_TREE, NULL, 0, 0);
    assert_non_null(buf);
    assert_string_equal(buf, tree_alldisabled);
    free(buf); buf = NULL;

    lys_features_enable(mod, "a");
    lys_print_mem(&buf, mod, LYS_OUT_TREE, NULL, 0, 0);
    assert_non_null(buf);
    assert_string_equal(buf, tree_a);
    free(buf); buf = NULL;

    lys_features_enable(mod, "b");
    lys_print_mem(&buf, mod, LYS_OUT_TREE, NULL, 0, 0);
    assert_non_null(buf);
    assert_string_equal(buf, tree_ab);
    free(buf); buf = NULL;

    lys_features_enable(mod, "aa");
    lys_print_mem(&buf, mod, LYS_OUT_TREE, NULL, 0, 0);
    assert_non_null(buf);
    assert_string_equal(buf, tree_abaa);
    free(buf); buf = NULL;

    lys_features_disable(mod, "a"); /* aa is also disabled by disabling a */
    lys_print_mem(&buf, mod, LYS_OUT_TREE, NULL, 0, 0);
    assert_non_null(buf);
    assert_string_equal(buf, tree_b);
    free(buf); buf = NULL;
}

static void
test_circle1(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a { if-feature \"b and c\"; }\n"
"  feature b { if-feature c; }\n"
"  feature c { if-feature \"a or b\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_CIRC_FEATURES);
}

static void
test_circle2(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a { if-feature \"a\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_CIRC_FEATURES);
}

static void
test_inval_expr1(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a;\n"
"  feature b;\n"
"  feature c { if-feature \"a xor b\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_INARG);
}

static void
test_inval_expr2(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a;\n"
"  feature b;\n"
"  feature c { if-feature \"\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_INARG);
}

static void
test_inval_expr3(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a;\n"
"  feature b;\n"
"  feature c { if-feature \"x\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_INRESOLV);
}

static void
test_inval_expr4(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a;\n"
"  feature b;\n"
"  feature c { if-feature \"a b\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_INARG);
}

static void
test_inval_expr5(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang = "module features {\n"
"  namespace \"urn:features\";\n"
"  prefix f;\n"
"  feature a;\n"
"  feature b;\n"
"  feature c { if-feature \"a and b\"; }}";

    assert_null(lys_parse_mem(ctx, yang, LYS_IN_YANG));
    assert_int_equal(ly_vecode(ctx), LYVE_INARG);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_fullset, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_fullset, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_circle1, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_circle2, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_inval_expr1, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_inval_expr2, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_inval_expr3, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_inval_expr4, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_inval_expr5, setup_ctx_yang, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
