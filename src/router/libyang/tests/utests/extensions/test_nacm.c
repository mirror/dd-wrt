/*
 * @file test_nacm.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for NACM extensions support
 *
 * Copyright (c) 2019-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

static int
setup(void **state)
{
    UTEST_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-netconf-acm", "2018-02-14", NULL));

    return 0;
}

static void
test_deny_all(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_container *cont;
    struct lysc_node_leaf *leaf;
    struct lysc_ext_instance *e;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:nacm:a; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "container a { nacm:default-deny-all; leaf aa {type string;}}"
            "leaf b {type string;}}";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(cont = (struct lysc_node_container *)mod->compiled->data);
    assert_non_null(leaf = (struct lysc_node_leaf *)cont->child);
    assert_non_null(e = &cont->exts[0]);
    assert_int_equal(LY_ARRAY_COUNT(cont->exts), 1);
    assert_int_equal(LY_ARRAY_COUNT(leaf->exts), 1); /* NACM extensions inherit */
    assert_ptr_equal(e->def, leaf->exts[0].def);
    assert_int_equal(1, *((uint8_t *)e->data)); /* plugin's value for default-deny-all */
    assert_null(cont->next->exts);

    /* ignored - valid with warning */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:nacm:b; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "nacm:default-deny-all;}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-all is allowed only in a data nodes, but it is placed in \"module\" statement.",
            "/b:{extension='nacm:default-deny-all'}");

    /* invalid */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "leaf l { type string; nacm:default-deny-all; nacm:default-deny-write;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is mixed with nacm:default-deny-all.",
            "/aa:l/{extension='nacm:default-deny-write'}");
}

static void
test_deny_write(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_container *cont;
    struct lysc_node_leaf *leaf;
    struct lysc_ext_instance *e;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:nacm:a; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "container a { nacm:default-deny-write; leaf aa {type string;}}"
            "leaf b {type string;}}";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(cont = (struct lysc_node_container *)mod->compiled->data);
    assert_non_null(leaf = (struct lysc_node_leaf *)cont->child);
    assert_non_null(e = &cont->exts[0]);
    assert_int_equal(LY_ARRAY_COUNT(cont->exts), 1);
    assert_int_equal(LY_ARRAY_COUNT(leaf->exts), 1); /* NACM extensions inherit */
    assert_ptr_equal(e->def, leaf->exts[0].def);
    assert_int_equal(2, *((uint8_t *)e->data)); /* plugin's value for default-deny-write */

    /* ignored - valid with warning */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:nacm:b; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "notification notif {nacm:default-deny-write;}}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is not allowed in notification statement.",
            "/b:notif/{extension='nacm:default-deny-write'}");

    /* invalid */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:nacm:aa; prefix en;"
            "import ietf-netconf-acm {revision-date 2018-02-14; prefix nacm;}"
            "leaf l { type string; nacm:default-deny-write; nacm:default-deny-write;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - NACM, version 1\": "
            "Extension nacm:default-deny-write is instantiated multiple times.",
            "/aa:l/{extension='nacm:default-deny-write'}");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_deny_all, setup),
        UTEST(test_deny_write, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
