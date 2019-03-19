/**
 * \file test_leaflist.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - various tests connected with leaflists
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
test_multdfltvalues_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *root;
    const char *yin_cfg = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x\"/>"
"  <prefix value=\"x\"/>"
"  <yang-version value=\"1.1\"/>"
"  <container name=\"x\">"
"    <leaf-list name=\"ll\">"
"      <type name=\"string\"/>"
"      <default value=\"a\"/>"
"      <default value=\"a\"/>"
"    </leaf-list>"
"  </container>"
"</module>";

    const char *yin_status_10 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x\"/>"
"  <prefix value=\"x\"/>"
"  <container name=\"x\">"
"    <leaf-list name=\"ll\">"
"      <config value=\"false\"/>"
"      <type name=\"string\"/>"
"      <default value=\"a\"/>"
"      <default value=\"a\"/>"
"    </leaf-list>"
"  </container>"
"</module>";

    const char *yin_status = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x\"/>"
"  <prefix value=\"x\"/>"
"  <yang-version value=\"1.1\"/>"
"  <container name=\"x\">"
"    <leaf-list name=\"ll\">"
"      <config value=\"false\"/>"
"      <type name=\"string\"/>"
"      <default value=\"a\"/>"
"      <default value=\"a\"/>"
"    </leaf-list>"
"  </container>"
"</module>";
    const char *xml = "<x xmlns=\"urn:x\"><ll>a</ll><ll>a</ll></x>";
    char *printed;

    /* only config leaflists must be unique, so in case of default data
     * the same value can be specified as default multiple times */

    assert_ptr_equal(lys_parse_mem(ctx, yin_cfg, LYS_IN_YIN), NULL);
    assert_ptr_equal(lys_parse_mem(ctx, yin_status_10, LYS_IN_YIN), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yin_status, LYS_IN_YIN), NULL);
    /* for validating complete data tree, we need data from ietf-yang-library */
    root = ly_ctx_info(ctx);
    assert_int_equal(lyd_validate(&root, LYD_OPT_DATA, ctx), 0);
    assert_ptr_not_equal(root, NULL); /* ietf-yang-library */
    assert_ptr_not_equal(root->next, NULL); /* added default nodes from module x */

    lyd_print_mem(&printed, root->prev, LYD_XML, LYP_WD_ALL); /* print only the default nodes from module x */
    assert_string_equal(printed, xml);

    free(printed);
    lyd_free_withsiblings(root);
}

static void
test_multdfltvalues_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *root;
    const char *yang_cfg = "module x {"
"  namespace urn:x;"
"  prefix x;"
"  yang-version 1.1;"
"  container x {"
"    leaf-list ll {"
"      type string;"
"      default a;"
"      default a;"
"    }"
"  }"
"}";
    const char *yang_status_10 = "module x {"
"  namespace urn:x;"
"  prefix x;"
"  container x {"
"    leaf-list ll {"
"      config false;"
"      type string;"
"      default a;"
"      default a;"
"    }"
"  }"
"}";
    const char *yang_status = "module x {"
"  namespace urn:x;"
"  prefix x;"
"  yang-version 1.1;"
"  container x {"
"    leaf-list ll {"
"      config false;"
"      type string;"
"      default a;"
"      default a;"
"    }"
"  }"
"}";
    const char *xml = "<x xmlns=\"urn:x\"><ll>a</ll><ll>a</ll></x>";
    char *printed;

    /* only config leaflists must be unique, so in case of default data
     * the same value can be specified as default multiple times */

    assert_ptr_equal(lys_parse_mem(ctx, yang_cfg, LYS_IN_YANG), NULL);
    assert_ptr_equal(lys_parse_mem(ctx, yang_status_10, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yang_status, LYS_IN_YANG), NULL);
    /* for validating complete data tree, we need data from ietf-yang-library */
    root = ly_ctx_info(ctx);
    assert_int_equal(lyd_validate(&root, LYD_OPT_DATA, ctx), 0);
    assert_ptr_not_equal(root, NULL); /* ietf-yang-library */
    assert_ptr_not_equal(root->next, NULL); /* added default nodes from module x */

    lyd_print_mem(&printed, root->prev, LYD_XML, LYP_WD_ALL); /* print only the default nodes from module x */
    assert_string_equal(printed, xml);

    free(printed);
    lyd_free_withsiblings(root);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_multdfltvalues_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_multdfltvalues_yang, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
